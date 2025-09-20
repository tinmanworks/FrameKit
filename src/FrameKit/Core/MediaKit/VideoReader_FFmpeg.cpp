// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/MediaKit/VideoReader_FFmpeg.cpp
// Author       : George Gil
// Created      : 2025-09-19
// Updated      : 2025-09-19
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//		Video reader implementation using FFmpeg
// =============================================================================

//#include "FrameKit/MediaKit/MediaKit.h"
//
//#if defined(FK_MEDIA_ENABLE_FFMPEG)
//extern "C" {
//#include <libavformat/avformat.h>
//#include <libavcodec/avcodec.h>
//#include <libswscale/swscale.h>
//#include <libswresample/swresample.h>
//#include <libavutil/frame.h>   // AV_FRAME_FLAG_KEY
//}
//#include <stdexcept>
//#include <string>
//#include <cstring>
//
//namespace FrameKit::MediaKit {
//
//    static PixelFormat choose_out_fmt(enum AVPixelFormat) { return PixelFormat::RGBA8; }
//
//    class FFmpegVideoReader final : public IVideoReader {
//    public:
//        ~FFmpegVideoReader() override { close(); }
//
//        DemuxInfo open(std::string_view p) override {
//            close();
//            path_ = std::string(p);
//
//            if (avformat_open_input(&fmt_, path_.c_str(), nullptr, nullptr) < 0)
//                throw std::runtime_error("avformat_open_input failed");
//            if (avformat_find_stream_info(fmt_, nullptr) < 0)
//                throw std::runtime_error("avformat_find_stream_info failed");
//
//            vstream_ = av_find_best_stream(fmt_, AVMEDIA_TYPE_VIDEO, -1, -1, &vcodec_, 0);
//            astream_ = av_find_best_stream(fmt_, AVMEDIA_TYPE_AUDIO, -1, -1, &acodec_, 0);
//
//            if (vstream_ >= 0) open_video();
//            if (astream_ >= 0) open_audio();
//
//            DemuxInfo info{};
//            if (vstream_ >= 0) info.video = vinfo_;
//            if (astream_ >= 0) info.audio = ainfo_;
//            info.durationSec = (fmt_->duration > 0) ? (double)fmt_->duration / AV_TIME_BASE : 0.0;
//            info.isSeekable = (fmt_->pb && (fmt_->pb->seekable & AVIO_SEEKABLE_NORMAL)) != 0;
//
//            eof_ = false;
//            // After open, force resync at first IDR
//            seekingDropUntilKey_ = true;
//            fastResync_ = false;
//            return info;
//        }
//
//        bool read(VideoFrame* vf, AudioFrame* af) override {
//            if (!fmt_) return false;
//
//            const bool wantV = vf != nullptr && vc_ != nullptr;
//            const bool wantA = af != nullptr && ac_ != nullptr;
//
//            AVPacket* pkt = av_packet_alloc();
//            if (!pkt) return false;
//
//            // Drain pending frames first
//            if (wantV && try_receive_video(vf)) { av_packet_free(&pkt); return true; }
//            if (wantA && try_receive_audio(af)) { av_packet_free(&pkt); return true; }
//
//            while (av_read_frame(fmt_, pkt) >= 0) {
//                if (wantV && pkt->stream_index == vstream_) {
//                    if (avcodec_send_packet(vc_, pkt) == 0) {
//                        // Drain all available frames for this packet
//                        if (try_receive_video(vf)) { av_packet_unref(pkt); av_packet_free(&pkt); return true; }
//                    }
//                }
//                else if (wantA && pkt->stream_index == astream_) {
//                    if (avcodec_send_packet(ac_, pkt) == 0) {
//                        if (try_receive_audio(af)) { av_packet_unref(pkt); av_packet_free(&pkt); return true; }
//                    }
//                }
//                av_packet_unref(pkt);
//            }
//
//            // EOF: flush once
//            if (!eof_) {
//                eof_ = true;
//                if (wantV) avcodec_send_packet(vc_, nullptr);
//                if (wantA) avcodec_send_packet(ac_, nullptr);
//                if (wantV && try_receive_video(vf)) { av_packet_free(&pkt); return true; }
//                if (wantA && try_receive_audio(af)) { av_packet_free(&pkt); return true; }
//            }
//
//            av_packet_free(&pkt);
//            return false;
//        }
//
//        bool seek(double s, bool exact) override {
//            if (!fmt_) return false;
//            eof_ = false;
//
//            // Prefer video stream time base if present
//            int vs = vstream_ >= 0 ? vstream_ : -1;
//            AVRational tb = vs >= 0 ? fmt_->streams[vs]->time_base : AVRational{ 1, AV_TIME_BASE };
//            int64_t ts = vs >= 0
//                ? static_cast<int64_t>(s / av_q2d(tb))
//                : static_cast<int64_t>(s * AV_TIME_BASE);
//            int flags = exact ? 0 : AVSEEK_FLAG_BACKWARD;
//
//            if (av_seek_frame(fmt_, vs, ts, flags) < 0) return false;
//
//            if (vc_) avcodec_flush_buffers(vc_);
//            if (ac_) avcodec_flush_buffers(ac_);
//
//            // Drop frames until the first keyframe after seek
//            seekingDropUntilKey_ = true;
//
//            // Speed resync by discarding non-reference frames temporarily
//            fastResync_ = true;
//            prevSkipFrame_ = vc_ ? vc_->skip_frame : AVDISCARD_DEFAULT;
//            if (vc_) vc_->skip_frame = static_cast<AVDiscard>(AVDISCARD_NONREF);
//
//            return true;
//        }
//
//        void close() override {
//            if (vc_) { avcodec_free_context(&vc_); vc_ = nullptr; }
//            if (ac_) { avcodec_free_context(&ac_); ac_ = nullptr; }
//            if (fmt_) { avformat_close_input(&fmt_); fmt_ = nullptr; }
//            if (sws_) { sws_freeContext(sws_); sws_ = nullptr; }
//            if (swr_) { swr_free(&swr_); swr_ = nullptr; }
//            swsSrcFmt_ = AV_PIX_FMT_NONE; swsW_ = swsH_ = 0;
//            vstream_ = astream_ = -1;
//            vcodec_ = nullptr; acodec_ = nullptr;
//            vinfo_ = {}; ainfo_ = {};
//            path_.clear();
//            eof_ = false;
//            seekingDropUntilKey_ = true;
//            fastResync_ = false;
//            prevSkipFrame_ = AVDISCARD_DEFAULT;
//        }
//
//    private:
//        // --- open helpers ---
//        void open_video() {
//            vc_ = avcodec_alloc_context3(vcodec_);
//            if (!vc_) throw std::runtime_error("alloc video codec ctx failed");
//            if (avcodec_parameters_to_context(vc_, fmt_->streams[vstream_]->codecpar) < 0)
//                throw std::runtime_error("video params->ctx failed");
//            if (avcodec_open2(vc_, vcodec_, nullptr) < 0)
//                throw std::runtime_error("avcodec_open2 video failed");
//
//            vinfo_.w = vc_->width; vinfo_.h = vc_->height;
//            vinfo_.fps = fps_of(fmt_->streams[vstream_]);
//            vinfo_.decodeFmt = choose_out_fmt(static_cast<AVPixelFormat>(vc_->pix_fmt));
//            vinfo_.cs = {};
//            ensure_sws(static_cast<AVPixelFormat>(vc_->pix_fmt), vc_->width, vc_->height);
//
//            // Prepare resync state on fresh open
//            seekingDropUntilKey_ = true;
//            fastResync_ = false;
//            prevSkipFrame_ = vc_->skip_frame;
//        }
//
//        void open_audio() {
//            ac_ = avcodec_alloc_context3(acodec_);
//            if (!ac_) throw std::runtime_error("alloc audio codec ctx failed");
//            if (avcodec_parameters_to_context(ac_, fmt_->streams[astream_]->codecpar) < 0)
//                throw std::runtime_error("audio params->ctx failed");
//            if (avcodec_open2(ac_, acodec_, nullptr) < 0)
//                throw std::runtime_error("avcodec_open2 audio failed");
//
//            ainfo_.sampleRate = ac_->sample_rate;
//            ainfo_.channels = ac_->ch_layout.nb_channels;
//            ainfo_.channelMask = static_cast<int64_t>(ac_->ch_layout.u.mask);
//
//            if (!swr_) {
//                int err = swr_alloc_set_opts2(
//                    &swr_,
//                    &ac_->ch_layout, AV_SAMPLE_FMT_FLT, ac_->sample_rate,              // dst: interleaved float32
//                    &ac_->ch_layout, static_cast<AVSampleFormat>(ac_->sample_fmt), ac_->sample_rate, // src: codec native
//                    0, nullptr);
//                if (err < 0 || swr_init(swr_) < 0)
//                    throw std::runtime_error("swr alloc/init failed");
//            }
//        }
//
//        static double fps_of(AVStream* st) {
//            AVRational r = av_guess_frame_rate(nullptr, st, nullptr);
//            if (r.num == 0 || r.den == 0) r = st->avg_frame_rate.num ? st->avg_frame_rate : st->r_frame_rate;
//            if (r.num == 0 || r.den == 0) return 0.0;
//            return static_cast<double>(r.num) / static_cast<double>(r.den);
//        }
//
//        static FramePTS to_pts(int64_t pts, AVRational tb) {
//            if (pts == AV_NOPTS_VALUE) return { 0,1 };
//            return { pts * static_cast<int64_t>(tb.num), tb.den };
//        }
//
//        // --- decode helpers ---
//        bool try_receive_video(VideoFrame* out) {
//            if (!vc_ || !out) return false;
//
//            for (;;) {
//                AVFrame* f = av_frame_alloc();
//                int ret = avcodec_receive_frame(vc_, f);
//                if (ret == AVERROR(EAGAIN)) { av_frame_free(&f); return false; }
//                if (ret == AVERROR_EOF) { av_frame_free(&f); return false; }
//                if (ret < 0) { av_frame_free(&f); return false; }
//
//                // Drop until keyframe after open/seek
//                if (seekingDropUntilKey_) {
//                    const bool isKey = (f->flags & AV_FRAME_FLAG_KEY) != 0 || f->pict_type == AV_PICTURE_TYPE_I;
//                    if (!isKey) { av_frame_free(&f); continue; }
//                    seekingDropUntilKey_ = false;
//                    if (fastResync_ && vc_) {
//                        vc_->skip_frame = prevSkipFrame_;
//                        fastResync_ = false;
//                    }
//                }
//
//                // Use best_effort_timestamp for correct order with B-frames
//                int64_t ts = (f->best_effort_timestamp != AV_NOPTS_VALUE) ? f->best_effort_timestamp : f->pts;
//                ensure_sws(static_cast<AVPixelFormat>(f->format), f->width, f->height);
//
//                VideoFrame vf;
//                vf.info = vinfo_;
//                vf.pts = to_pts(ts, fmt_->streams[vstream_]->time_base);
//                vf.planes.resize(1);
//                vf.planes[0].resize(static_cast<size_t>(f->width) * static_cast<size_t>(f->height) * 4u);
//
//                uint8_t* dst[4] = { vf.planes[0].data(), nullptr, nullptr, nullptr };
//                int      dstln[4] = { f->width * 4, 0, 0, 0 };
//                sws_scale(sws_, f->data, f->linesize, 0, f->height, dst, dstln);
//
//                av_frame_free(&f);
//                *out = std::move(vf);
//                return true;
//            }
//        }
//
//        bool try_receive_audio(AudioFrame* out) {
//            if (!ac_ || !out) return false;
//            AVFrame* f = av_frame_alloc();
//            int ret = avcodec_receive_frame(ac_, f);
//            if (ret == 0) {
//                // Rebuild swr if layout/format/rate changed
//                if (!swr_
//                    || f->sample_rate != ac_->sample_rate
//                    || f->ch_layout.nb_channels != ac_->ch_layout.nb_channels
//                    || f->format != ac_->sample_fmt) {
//                    if (swr_) { swr_free(&swr_); swr_ = nullptr; }
//                    int err = swr_alloc_set_opts2(
//                        &swr_,
//                        &ac_->ch_layout, AV_SAMPLE_FMT_FLT, ac_->sample_rate,
//                        &ac_->ch_layout, (AVSampleFormat)f->format, f->sample_rate,
//                        0, nullptr);
//                    if (err < 0 || swr_init(swr_) < 0) { av_frame_free(&f); return false; }
//                }
//
//                AudioFrame af;
//                af.info = ainfo_;
//                af.pts = to_pts((f->best_effort_timestamp != AV_NOPTS_VALUE) ? f->best_effort_timestamp : f->pts,
//                    fmt_->streams[astream_]->time_base);
//
//                // Capacity in samples per channel
//                int out_cap = swr_get_out_samples(swr_, f->nb_samples);
//                if (out_cap < f->nb_samples) out_cap = f->nb_samples;
//
//                const int dst_bytes = out_cap * af.info.channels * (int)sizeof(float);
//                af.pcmF32.resize(dst_bytes);
//                uint8_t* dst_data = af.pcmF32.data();
//
//                int out_samples = swr_convert(swr_, &dst_data, out_cap,
//                    const_cast<const uint8_t**>(f->data), f->nb_samples);
//                if (out_samples < 0) { av_frame_free(&f); return false; }
//                // Trim to produced size
//                af.pcmF32.resize(out_samples * af.info.channels * (int)sizeof(float));
//
//                av_frame_free(&f);
//                *out = std::move(af);
//                return true;
//            }
//            av_frame_free(&f);
//            return false;
//        }
//
//
//        void ensure_sws(AVPixelFormat srcFmt, int w, int h) {
//            if (sws_ && (srcFmt == swsSrcFmt_) && w == swsW_ && h == swsH_) return;
//            if (sws_) { sws_freeContext(sws_); sws_ = nullptr; }
//            sws_ = sws_getContext(w, h, srcFmt, w, h, AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);
//            swsSrcFmt_ = srcFmt; swsW_ = w; swsH_ = h;
//        }
//
//    private:
//        std::string       path_{};
//        AVFormatContext* fmt_{ nullptr };
//        int               vstream_{ -1 }, astream_{ -1 };
//        const AVCodec* vcodec_{ nullptr };
//        const AVCodec* acodec_{ nullptr };
//        AVCodecContext* vc_{ nullptr };
//        AVCodecContext* ac_{ nullptr };
//        SwsContext* sws_{ nullptr };
//        AVPixelFormat     swsSrcFmt_{ AV_PIX_FMT_NONE };
//        int               swsW_{ 0 }, swsH_{ 0 };
//        SwrContext* swr_{ nullptr };
//        VideoStreamInfo   vinfo_{};
//        AudioStreamInfo   ainfo_{};
//        bool              eof_{ false };
//
//        // resync state
//        bool       seekingDropUntilKey_{ true };
//        bool       fastResync_{ false };
//        AVDiscard  prevSkipFrame_{ AVDISCARD_DEFAULT };
//    };
//
//    // Unique maker for central factory
//    std::unique_ptr<IVideoReader> MakeVideoReader_FFmpeg() {
//        return std::make_unique<FFmpegVideoReader>();
//    }
//
//} // namespace FrameKit::MediaKit
//#endif // FK_MEDIA_ENABLE_FFMPEG