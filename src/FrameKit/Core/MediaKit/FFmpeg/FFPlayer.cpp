// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/MediaKit/FFmpeg/FFPlayer.cpp
// Author       : George Gil
// Created      : 2025-09-19
// Updated      : 2025-09-19
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//		FFmpeg-based media player implementation.
// =============================================================================
#include "FFPlayer.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/pixfmt.h>
}

using Clock = std::chrono::steady_clock;
static inline double NowSec() {
    return std::chrono::duration<double>(Clock::now().time_since_epoch()).count();
}
static inline double Since(const Clock::time_point& tp) {
    return std::chrono::duration<double>(Clock::now() - tp).count();
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


namespace FrameKit::MediaKit {
    std::unique_ptr<IPlayer> MakePlayer_FFmpeg() { return std::make_unique<FFPlayer>(); }
}

using namespace FrameKit::MediaKit;

bool FFPlayer::open(std::string_view path, const PlayerConfig& cfg) {
    close();
    state_ = PlayerState::Opening;
    cfg_ = cfg;
    {
        std::lock_guard<std::mutex> lk(rdrMtx_);
        rdr_ = std::make_unique<FFVideoReader>();
        readerInfo_ = rdr_->open(path);
    }
    if (!readerInfo_.video) { 
        rdr_.reset(); 
        state_ = PlayerState::Error; 
        return false; 
    }
    quit_ = false; 
    clock_ = 0.0; 
    rate_ = 1.0;
    paused_ = true;
    baseMedia_ = 0.0;
    baseWall_ = Clock::now();
    lastPTS_ = 0.0;
    state_ = PlayerState::Paused;
    thRead_ = std::thread(&FFPlayer::demuxDecodeThread, this);
    thPresent_ = std::thread(&FFPlayer::presentThread, this);
    return true;
}

void FFPlayer::close() {
    quit_ = true; cv_.notify_all();
    if (thRead_.joinable()) thRead_.join();
    if (thPresent_.joinable()) thPresent_.join();
    {
        std::lock_guard<std::mutex> lk(rdrMtx_);
        if (rdr_) rdr_->close();
        rdr_.reset();
    }
    std::scoped_lock lk(m_); vq_.clear();
    state_ = PlayerState::Idle;
}

void FFPlayer::play() {
    if (!paused_) return;
    // resume: pin wall clock to current media time
    baseWall_ = Clock::now();
    paused_ = false;
    state_ = PlayerState::Playing;
    cv_.notify_all();
}

void FFPlayer::pause() {
    if (paused_) return;
    // capture current running time into baseMedia_
    baseMedia_ = time();
    paused_ = true;
    state_ = PlayerState::Paused;
    cv_.notify_all();
}

void FFPlayer::stop() {
    pause();
    seek(0.0, false);
    pause();
    state_ = PlayerState::Stopped;
}

bool FFPlayer::setRate(double r) {
    if (r <= 0.0) return false;
    // retime from current time
    baseMedia_ = time();
    baseWall_ = Clock::now();
    rate_ = r;
    return true;
}

double FFPlayer::getRate() {
    return rate_;
}

bool FFPlayer::seek(double s, bool exact) {
    pause();
    {
        std::lock_guard<std::mutex> lk(m_);
        vq_.clear();
    }
    {
        std::lock_guard<std::mutex> lk(rdrMtx_);   // <-- serialize with read()
        lastPTS_ = s;
        baseMedia_ = s;
        baseWall_ = Clock::now();
        if (rdr_) rdr_->seek(s, exact);
    }
    cv_.notify_all();
    play();
    return true;
}

double FFPlayer::time() const {
    if (paused_) return baseMedia_;
    return baseMedia_ + rate_ * std::chrono::duration<double>(Clock::now() - baseWall_).count();
}

bool FFPlayer::getVideo(VideoFrame& out) {
    std::unique_lock lk(m_);
    if (vq_.empty()) return false;
    out = std::move(vq_.front()); vq_.pop_front();
    return true;
}

void FFPlayer::setVideoSink(VideoSink s) {
    std::lock_guard<std::mutex> lk(sinkMtx);
    sinkV_ = std::move(s);
}

void FFPlayer::demuxDecodeThread() {
    VideoFrame vf;
    SwsState sws;
    std::vector<uint8_t> rgba;

    for (;;) {
        if (quit_) return;

        if (paused_) {
            std::unique_lock lk(m_);
            cv_.wait(lk, [&] { return quit_ || !paused_; });
            if (quit_) return;
        }

        {
            std::unique_lock lk(m_);
            if ((int)vq_.size() >= cfg_.videoQueue) { lk.unlock(); std::this_thread::sleep_for(std::chrono::milliseconds(2)); continue; }
        }

        bool ok;
        {
            std::lock_guard<std::mutex> lk(rdrMtx_);
            ok = rdr_->read(&vf, nullptr);
        }
        if (!ok) {
            if (loop_) { rdr_->seek(0.0, false); continue; }
            state_ = PlayerState::Ended;
            pause(); // freeze time() at current baseMedia_
            continue;
        }

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

        {
            std::scoped_lock lk(m_);
            vq_.push_back(std::move(vf));
        }
        cv_.notify_all();
    }
}

void FFPlayer::presentThread() {
    const double epsilon = 1.0 / 1000.0; // 1 ms
    for (;;) {
        if (quit_) return;

        // wait if paused
        if (paused_) {
            std::unique_lock lk(m_);
            cv_.wait(lk, [&] { return quit_ || !paused_; });
            if (quit_) return;
        }

        VideoFrame fr;
        {
            std::unique_lock lk(m_);
            if (vq_.empty()) { lk.unlock(); std::this_thread::sleep_for(std::chrono::milliseconds(1)); continue; }
            fr = vq_.front();
        }

        double pts = PtsSeconds(fr.pts);
        double now = time();

        if (pts <= now + epsilon) {
            // present now
            {
                std::unique_lock lk(m_);
                if (!vq_.empty()) vq_.pop_front();
            }
            lastPTS_ = pts;
            VideoSink local;
            {
                std::lock_guard<std::mutex> lk(sinkMtx);
                local = sinkV_;
            }
            if (local) local(fr);
            continue;
        }

        // Sleep only by the delta; no extra rate scaling here.
        double waitSec = (pts - now);
        if (waitSec > 0.020) waitSec = 0.020; // cap sleep granularity
        if (waitSec < 0.0) waitSec = 0.0;
        std::this_thread::sleep_for(std::chrono::duration<double>(waitSec));
    }
}

