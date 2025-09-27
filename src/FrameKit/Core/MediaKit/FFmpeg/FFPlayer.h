// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/MediaKit/FFmpeg/FFPlayer.h
// Author       : George Gil
// Created      : 2025-09-19
// Updated      : 2025-09-19
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//		FFmpeg-based media player implementation.
// =============================================================================
#pragma once
#include "FrameKit/MediaKit/MediaKit.h"
#include "FFVideoReader.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
namespace FrameKit::MediaKit {
    class FFPlayer final : public IPlayer {
    public:
        bool open(std::string_view path, const PlayerConfig& cfg) override;
        void close() override;
        void play() override; void pause() override; void stop() override;
        bool seek(double s, bool exact = false) override;
        bool setRate(double) override;
        double getRate() override;
        void setLoop(bool v) override { loop_ = v; }
        PlayerState state() const override { return state_; }
        MediaInfo info() const override { return { readerInfo_ }; }
        double time() const override;
        bool getVideo(VideoFrame& out) override;
        bool getAudio(AudioFrame&) override { return false; }
        void setVideoSink(VideoSink s) override;
        void setAudioSink(AudioSink) override {}
        void setExternalTime(double t) override { extClock_ = t; }
    private:
        void demuxDecodeThread();
        void presentThread();

        PlayerConfig cfg_{};
        PlayerState state_{ PlayerState::Idle };
        std::unique_ptr<FFVideoReader> rdr_;
        DemuxInfo readerInfo_{};
        std::thread thRead_, thPresent_;
        std::atomic<bool> quit_{ false };
        std::deque<VideoFrame> vq_;
        std::mutex rdrMtx_;   // guards rdr_->open/read/seek/close
        std::mutex sinkMtx;
        mutable std::mutex m_;
        std::condition_variable cv_;
        double clock_{ 0.0 }, extClock_{ 0.0 };
        double baseMedia_{ 0.0 };
        std::chrono::steady_clock::time_point baseWall_{};
        double lastPTS_{ 0.0 };     // last presented PTS
        double rate_{ 1.0 };
        bool loop_{ false }, paused_{ true };
        VideoSink sinkV_{};
    };
}
