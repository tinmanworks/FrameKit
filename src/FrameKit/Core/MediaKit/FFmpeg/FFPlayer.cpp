// FFPlayer.cpp
#include "FFPlayer.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/pixfmt.h>
}

static AVPixelFormat AvFromFk(FrameKit::MediaKit::PixelFormat f) {
    using PF = FrameKit::MediaKit::PixelFormat;
    switch (f) {
    case PF::RGBA8:   return AV_PIX_FMT_RGBA;
    case PF::BGRA8:   return AV_PIX_FMT_BGRA;
    case PF::YUV420P: return AV_PIX_FMT_YUV420P;
    case PF::NV12:    return AV_PIX_FMT_NV12;
    case PF::P010:    return AV_PIX_FMT_P010;
    default:          return AV_PIX_FMT_NONE;
    }
}

struct SwsState {
    SwsContext* ctx{ nullptr };
    int w{ 0 }, h{ 0 };
    AVPixelFormat src{ AV_PIX_FMT_NONE };
    AVPixelFormat dst{ AV_PIX_FMT_RGBA };
    ~SwsState() { if (ctx) sws_freeContext(ctx); }
};

static bool ConvertToRGBA(const FrameKit::MediaKit::VideoFrame& in,
    FrameKit::MediaKit::PixelFormat dstFkFmt,
    std::vector<uint8_t>& outRGBA,
    SwsState& st)
{
    using namespace FrameKit::MediaKit;
    const int w = in.info.w, h = in.info.h;
    const AVPixelFormat src = AvFromFk(in.info.decodeFmt);
    const AVPixelFormat dst = (dstFkFmt == PixelFormat::BGRA8) ? AV_PIX_FMT_BGRA : AV_PIX_FMT_RGBA;

    if (!st.ctx || st.w != w || st.h != h || st.src != src || st.dst != dst) {
        if (st.ctx) sws_freeContext(st.ctx);
        st.ctx = sws_getContext(w, h, src, w, h, dst, SWS_BILINEAR, nullptr, nullptr, nullptr);
        st.w = w; st.h = h; st.src = src; st.dst = dst;
        if (!st.ctx) return false;
    }

    const uint8_t* srcData[4] = { nullptr,nullptr,nullptr,nullptr };
    int srcStride[4] = { 0,0,0,0 };

    switch (in.info.decodeFmt) {
    case PixelFormat::YUV420P:
        // Tight-packed planes
        srcData[0] = in.planes[0].data(); srcStride[0] = w;
        srcData[1] = in.planes[1].data(); srcStride[1] = w / 2;
        srcData[2] = in.planes[2].data(); srcStride[2] = w / 2;
        break;
    case PixelFormat::NV12:
        srcData[0] = in.planes[0].data(); srcStride[0] = w;
        srcData[1] = in.planes[1].data(); srcStride[1] = w;
        break;
    case PixelFormat::P010:
        srcData[0] = in.planes[0].data(); srcStride[0] = 2 * w;
        srcData[1] = in.planes[1].data(); srcStride[1] = 2 * w;
        break;
    case PixelFormat::RGBA8:
    case PixelFormat::BGRA8:
        // already RGBA/BGRA
        outRGBA = in.planes[0];
        return true;
    default:
        return false;
    }

    outRGBA.resize(size_t(w) * size_t(h) * 4);
    uint8_t* dstData[4] = { outRGBA.data(), nullptr, nullptr, nullptr };
    int dstStride[4] = { w * 4, 0, 0, 0 };

    int r = sws_scale(st.ctx, srcData, srcStride, 0, h, dstData, dstStride);
    return r > 0;
}


using namespace FrameKit::MediaKit;

namespace FrameKit::MediaKit {
    std::unique_ptr<IPlayer> MakePlayer_FFmpeg() { return std::make_unique<FFPlayer>(); }
}


bool FFPlayer::open(std::string_view path, const PlayerConfig& cfg) {
    close();
    cfg_ = cfg;
    rdr_ = std::make_unique<FFVideoReader>();
    readerInfo_ = rdr_->open(path);
    if (!readerInfo_.video) { rdr_.reset(); state_ = PlayerState::Error; return false; }
    quit_ = false; paused_ = true; clock_ = 0.0; state_ = PlayerState::Paused;
    thRead_ = std::thread(&FFPlayer::demuxDecodeThread, this);
    thPresent_ = std::thread(&FFPlayer::presentThread, this);
    return true;
}
void FFPlayer::close() {
    quit_ = true; cv_.notify_all();
    if (thRead_.joinable()) thRead_.join();
    if (thPresent_.joinable()) thPresent_.join();
    rdr_.reset();
    std::scoped_lock lk(m_); vq_.clear();
    state_ = PlayerState::Idle;
}
void FFPlayer::play() { paused_ = false; state_ = PlayerState::Playing; cv_.notify_all(); }
void FFPlayer::pause() { paused_ = true;  state_ = PlayerState::Paused;  cv_.notify_all(); }
void FFPlayer::stop() { pause(); seek(0.0, false); }
bool FFPlayer::seek(double s, bool exact) {
    std::scoped_lock lk(m_);
    vq_.clear();
    clock_ = s;
    bool ok = rdr_ && rdr_->seek(s, exact);
    cv_.notify_all();
    return ok;
}
bool FFPlayer::getVideo(VideoFrame& out) {
    std::unique_lock lk(m_);
    if (vq_.empty()) return false;
    out = std::move(vq_.front()); vq_.pop_front();
    return true;
}
void FFPlayer::demuxDecodeThread() {
    VideoFrame vf;
    SwsState sws;                      // <-- add state here
    std::vector<uint8_t> rgba;         // <-- temp buffer

    for (;;) {
        if (quit_) return;
        if (paused_) { std::unique_lock lk(m_); cv_.wait(lk, [&] { return quit_ || !paused_; }); if (quit_) return; }

        {
            std::unique_lock lk(m_);
            if ((int)vq_.size() >= cfg_.videoQueue) { lk.unlock(); std::this_thread::sleep_for(std::chrono::milliseconds(2)); continue; }
        }

        bool ok = rdr_->read(&vf, nullptr);
        if (!ok) { if (loop_) { rdr_->seek(0.0, false); continue; } state_ = PlayerState::Ended; paused_ = true; continue; }

        // Enforce requested output format
        if (cfg_.outFmt == PixelFormat::RGBA8 || cfg_.outFmt == PixelFormat::BGRA8) {
            if (!(vf.info.decodeFmt == PixelFormat::RGBA8 || vf.info.decodeFmt == PixelFormat::BGRA8)) {
                if (ConvertToRGBA(vf, cfg_.outFmt, rgba, sws)) {
                    vf.info.decodeFmt = cfg_.outFmt;
                    vf.planes.clear();
                    vf.planes.emplace_back(std::move(rgba));
                }
                else {
                    // fall through without conversion; UI may warn
                }
            }
        }

        { std::scoped_lock lk(m_); vq_.push_back(std::move(vf)); }
        cv_.notify_all();
    }
}

void FFPlayer::presentThread() {
    for (;;) {
        if (quit_) return;
        if (paused_) { std::unique_lock lk(m_); cv_.wait(lk, [&] { return quit_ || !paused_; }); if (quit_) return; }
        VideoFrame fr;
        {
            std::unique_lock lk(m_);
            if (vq_.empty()) { lk.unlock(); std::this_thread::sleep_for(std::chrono::milliseconds(1)); continue; }
            fr = vq_.front();
            double t = PtsSeconds(fr.pts);
            double now = clock_;
            if (t > now) { lk.unlock(); std::this_thread::sleep_for(std::chrono::milliseconds(1)); continue; }
            vq_.pop_front();
        }
        clock_ = PtsSeconds(fr.pts);
        if (sinkV_) sinkV_(fr); // push model
        // or have app call getVideo() in its render loop.
    }
}
