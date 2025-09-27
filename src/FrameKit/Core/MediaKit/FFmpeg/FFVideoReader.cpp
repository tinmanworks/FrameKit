// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/MediaKit/FFmpeg/FFVideoReader.cpp
// Author       : George Gil
// Created      : 2025-09-19
// Updated      : 2025-09-19
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//		FFmpeg-based video reader implementation.
// =============================================================================
#include "FFVideoReader.h"
using namespace FrameKit::MediaKit;
using namespace FrameKit::MediaKit::FF;

namespace FrameKit::MediaKit {
    std::unique_ptr<IVideoReader> MakeVideoReader_FFmpeg() {
        return std::make_unique<FFVideoReader>();
    }
}

DemuxInfo FFVideoReader::open(std::string_view path) {
    close();
    avformat_open_input(&fmt_, std::string(path).c_str(), nullptr, nullptr);
    avformat_find_stream_info(fmt_, nullptr);
    vs_ = av_find_best_stream(fmt_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    as_ = av_find_best_stream(fmt_, AVMEDIA_TYPE_AUDIO, -1, vs_, nullptr, 0);

    if (vs_ >= 0) {
        auto* vp = fmt_->streams[vs_]->codecpar;
        const AVCodec* vdec = avcodec_find_decoder(vp->codec_id);
        vctx_ = avcodec_alloc_context3(vdec);
        avcodec_parameters_to_context(vctx_, vp);
        avcodec_open2(vctx_, vdec, nullptr);

        VideoStreamInfo vi{};
        vi.w = vctx_->width; vi.h = vctx_->height;
        vi.fps = av_q2d(av_guess_frame_rate(fmt_, fmt_->streams[vs_], nullptr));
        vi.decodeFmt = MapPixFmt(vctx_->pix_fmt);
        vi.cs = { MapPrimaries(vctx_->color_primaries),
                  MapTRC(vctx_->color_trc),
                  vctx_->color_range == AVCOL_RANGE_JPEG };
        info_.video = vi;
    }
    if (as_ >= 0) {
        auto* ap = fmt_->streams[as_]->codecpar;
        const AVCodec* adec = avcodec_find_decoder(ap->codec_id);
        actx_ = avcodec_alloc_context3(adec);
        avcodec_parameters_to_context(actx_, ap);
        avcodec_open2(actx_, adec, nullptr);

        AudioStreamInfo ai{};
        ai.sampleRate = actx_->sample_rate;

#if LIBAVUTIL_VERSION_MAJOR >= 57  // new channel layout API (FFmpeg 5+)
        ai.channels = actx_->ch_layout.nb_channels;
        ai.channelMask = (actx_->ch_layout.order == AV_CHANNEL_ORDER_NATIVE)
            ? (int64_t)actx_->ch_layout.u.mask
            : 0;
#else                                 // legacy fields (FFmpeg 4.x)
        ai.channels = actx_->channels;
        ai.channelMask = (int64_t)actx_->channel_layout;
#endif

        info_.audio = ai;
    }
    info_.durationSec = (fmt_->duration > 0) ? (fmt_->duration / (double)AV_TIME_BASE) : 0.0;
    info_.isSeekable = (fmt_->pb && (fmt_->pb->seekable & AVIO_SEEKABLE_NORMAL)) != 0;

    pkt_ = av_packet_alloc();
    frm_ = av_frame_alloc();
    return info_;
}

bool FFVideoReader::read(VideoFrame* vf, AudioFrame* af) {
    // Simple: deliver video frames only first. Extend to audio later.
    while (av_read_frame(fmt_, pkt_) >= 0) {
        if (pkt_->stream_index != vs_) { av_packet_unref(pkt_); continue; }
        avcodec_send_packet(vctx_, pkt_); av_packet_unref(pkt_);
        for (;;) {
            int r = avcodec_receive_frame(vctx_, frm_);
            if (r == AVERROR(EAGAIN)) break;
            if (r == AVERROR_EOF) return false;

            auto* st = fmt_->streams[vs_];
            int64_t ts = (frm_->best_effort_timestamp != AV_NOPTS_VALUE) ?
                frm_->best_effort_timestamp : frm_->pts;
            FramePTS pts = ToPTS(ts, st->time_base);

            VideoFrame out{};
            out.info = *info_.video;
            out.pts = pts;

            // Export planes as-is (NV12/YUV420P preferred).
            out.planes.clear();
            int planes = av_pix_fmt_desc_get((AVPixelFormat)frm_->format)->nb_components;
            // Handle common formats explicitly
            if (frm_->format == AV_PIX_FMT_NV12) {
                out.planes.resize(2);
                // Y
                out.planes[0].assign(frm_->data[0], frm_->data[0] + frm_->linesize[0] * frm_->height);
                // UV
                out.planes[1].assign(frm_->data[1], frm_->data[1] + frm_->linesize[1] * (frm_->height / 2));
            }
            else if (frm_->format == AV_PIX_FMT_YUV420P) {
                out.planes.resize(3);
                int w = frm_->width, h = frm_->height;
                // copy each plane row-wise to pack tightly
                auto copyPlane = [&](int pi, int pw, int ph) {
                    out.planes[pi].resize(pw * ph);
                    for (int y = 0; y < ph; ++y)
                        memcpy(out.planes[pi].data() + y * pw, frm_->data[pi] + y * frm_->linesize[pi], pw);
                    };
                copyPlane(0, w, h);
                copyPlane(1, w / 2, h / 2);
                copyPlane(2, w / 2, h / 2);
            }
            else {
                // Convert to RGBA for simplicity
                sws_ = sws_getCachedContext(sws_, frm_->width, frm_->height, (AVPixelFormat)frm_->format,
                    frm_->width, frm_->height, AV_PIX_FMT_RGBA,
                    SWS_BILINEAR, nullptr, nullptr, nullptr);
                std::vector<uint8_t> rgba(frm_->width * frm_->height * 4);
                uint8_t* dst[4] = { rgba.data(), nullptr, nullptr, nullptr };
                int dstStride[4] = { frm_->width * 4, 0,0,0 };
                sws_scale(sws_, frm_->data, frm_->linesize, 0, frm_->height, dst, dstStride);
                out.info.decodeFmt = PixelFormat::RGBA8;
                out.planes = { std::move(rgba) };
            }

            av_frame_unref(frm_);
            if (vf) { *vf = std::move(out); return true; }
        }
    }
    // Drain decoder at EOF
    avcodec_send_packet(vctx_, nullptr);
    while (avcodec_receive_frame(vctx_, frm_) == 0) {
        // same as above if needed
        av_frame_unref(frm_);
        if (vf) return true; // fill properly if you implement drain output
    }
    return false;
}

bool FFVideoReader::seek(double seconds, bool exact) {
    if (!info_.isSeekable || vs_ < 0) return false;
    AVRational tb = fmt_->streams[vs_]->time_base;
    int64_t ts = llround(seconds / av_q2d(tb));
    int flags = exact ? 0 : AVSEEK_FLAG_BACKWARD;
    if (av_seek_frame(fmt_, vs_, ts, flags) < 0) return false;
    if (vctx_) avcodec_flush_buffers(vctx_);
    if (actx_) avcodec_flush_buffers(actx_);
    return true;
}

void FFVideoReader::close() {
    if (sws_) { sws_freeContext(sws_); sws_ = nullptr; }
    if (frm_) { av_frame_free(&frm_); }
    if (pkt_) { av_packet_free(&pkt_); }
    if (vctx_) { avcodec_free_context(&vctx_); }
    if (actx_) { avcodec_free_context(&actx_); }
    if (fmt_) { avformat_close_input(&fmt_); }
    vs_ = as_ = -1; info_ = {};
}
