// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/MediaKit/Player_FFmpeg.cpp
// Author       : George Gil
// Created      : 2025-09-19
// Updated      : 2025-09-19
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//		Player implementation using FFmpeg
// =============================================================================

//#include "FrameKit/MediaKit/MediaKit.h"
//
//#if defined(FK_MEDIA_ENABLE_FFMPEG)
//#include <chrono>
//#include <condition_variable>
//#include <deque>
//#include <mutex>
//#include <thread>
//#include <atomic>
//#include <stdexcept>
//
//namespace FrameKit::MediaKit {
//
//    class SimplePullPlayer final : public IPlayer {
//    public:
//        ~SimplePullPlayer() override { close(); }
//
//        bool open(std::string_view path, const PlayerConfig& cfg) override {
//            close();
//            cfg_ = cfg;
//            state_ = PlayerState::Opening;
//
//            pause_io_ = true;
//            {
//                std::lock_guard<std::mutex> rl(r_);
//                reader_ = CreateVideoReader(VideoBackend::FFmpeg);
//                demux_ = reader_->open(path);
//            }
//            pause_io_ = false; cv_.notify_all();
//
//            haveV_ = demux_.video.has_value();
//            haveA_ = demux_.audio.has_value();
//
//            paused_ = true;
//            startWall_ = Clock::now();
//            startMedia_ = 0.0;
//            externalTime_ = 0.0;
//
//            quit_ = false;
//            decodeTh_ = std::thread(&SimplePullPlayer::decodeLoop, this);
//            state_ = PlayerState::Paused;
//            return true;
//        }
//
//        void close() override {
//            quit_ = true; cv_.notify_all();
//            if (decodeTh_.joinable()) decodeTh_.join();
//
//            pause_io_ = true;
//            { std::lock_guard<std::mutex> rl(r_); reader_.reset(); }
//            pause_io_ = false;
//
//            std::lock_guard<std::mutex> lk(m_);
//            vq_.clear(); aq_.clear();
//            state_ = PlayerState::Idle;
//        }
//
//        // ---- Control ----
//        void play() override {
//            if (state_ == PlayerState::Opening || state_ == PlayerState::Error || state_ == PlayerState::Idle) return;
//            if (paused_) {
//                paused_ = false;
//                startWall_ = Clock::now();
//                state_ = PlayerState::Playing;
//            }
//        }
//
//        void pause() override {
//            if (!paused_) {
//                startMedia_ = time(); // freeze at current time
//                paused_ = true;
//                state_ = PlayerState::Paused;
//            }
//        }
//
//        void stop() override {
//            pause();
//            seek(0.0, false);
//            state_ = PlayerState::Stopped;
//        }
//
//        bool seek(double s, bool exact = false) override {
//            if (!reader_) return false;
//
//            pause_io_ = true;                // stop decode loop
//            {
//                std::lock_guard<std::mutex> lk(m_);
//                vq_.clear(); aq_.clear();
//            }
//            bool ok = false;
//            {
//                std::lock_guard<std::mutex> rl(r_);
//                ok = reader_->seek(s, exact);
//            }
//            startMedia_ = s;
//            startWall_ = Clock::now();
//            eofSeen_ = false;
//
//            pause_io_ = false;               // resume
//            cv_.notify_all();
//            return ok;
//        }
//
//        bool setRate(double r) override {
//            if (r <= 0.0) return false;
//            // adjust timeline so perceived time stays continuous
//            const double t = time();
//            rate_ = r;
//            startMedia_ = t;
//            startWall_ = Clock::now();
//            return true;
//        }
//
//        void setLoop(bool loop) override { loop_ = loop; }
//
//        // ---- Query ----
//        PlayerState state() const override { return state_; }
//        MediaInfo   info()  const override { return MediaInfo{ demux_ }; }
//
//        double time() const override {
//            if (cfg_.clockMode == ClockMode::External) return externalTime_;
//            if (paused_) return startMedia_;
//            using namespace std::chrono;
//            const double wall = duration<double>(Clock::now() - startWall_).count();
//            return startMedia_ + wall * rate_;
//        }
//
//        // ---- Pull delivery ----
//        bool getVideo(VideoFrame& out) override {
//            if (!haveV_) return false;
//            const double now = time();
//            std::lock_guard<std::mutex> lk(m_);
//            if (vq_.empty()) return false;
//
//            // Only release if due
//            const double pts = PtsSeconds(vq_.front().pts);
//            if (pts <= now + 0.001) { // small epsilon
//                out = std::move(vq_.front());
//                vq_.pop_front();
//                return true;
//            }
//            return false;
//        }
//
//        bool getAudio(AudioFrame& out) override {
//            if (!haveA_) return false;
//            std::lock_guard<std::mutex> lk(m_);
//            if (aq_.empty()) return false;
//            // Audio is usually pushed to device independently. Provide next chunk immediately.
//            out = std::move(aq_.front());
//            aq_.pop_front();
//            return true;
//        }
//
//        // ---- Push delivery (optional) ----
//        void setVideoSink(VideoSink s) override {
//            std::lock_guard<std::mutex> lk(m_);
//            vsink_ = std::move(s);
//        }
//        void setAudioSink(AudioSink s) override {
//            std::lock_guard<std::mutex> lk(m_);
//            asink_ = std::move(s);
//        }
//
//        void setExternalTime(double tSeconds) override {
//            externalTime_ = tSeconds;
//        }
//
//    private:
//        using Clock = std::chrono::steady_clock;
//
//        void decodeLoop() {
//            while (!quit_) {
//                if (pause_io_) {           // hold until seek completes
//                    std::unique_lock<std::mutex> lk(m_);
//                    cv_.wait_for(lk, std::chrono::milliseconds(2), [&] { return quit_ || !pause_io_; });
//                    continue;
//                }
//                {
//                    std::unique_lock<std::mutex> lk(m_);
//                    cv_.wait_for(lk, std::chrono::milliseconds(2), [&] {
//                        return quit_ || vq_.size() < (size_t)cfg_.videoQueue
//                            || aq_.size() < (size_t)cfg_.audioQueue;
//                        });
//                    if (quit_) break;
//                }
//
//                bool produced = false;
//
//                if (haveV_) {
//                    std::lock_guard<std::mutex> rl(r_);
//                    VideoFrame vf;
//                    if (reader_ && reader_->read(&vf, nullptr)) { produced = true; deliverVideo(std::move(vf)); }
//                }
//                if (haveA_) {
//                    std::lock_guard<std::mutex> rl(r_);
//                    AudioFrame af;
//                    if (reader_ && reader_->read(nullptr, &af)) { produced = true; deliverAudio(std::move(af)); }
//                }
//
//                if (!produced) {
//                    eofSeen_ = true;
//                    if (loop_ && demux_.durationSec > 0.0) { (void)seek(0.0, false); continue; }
//                    state_ = PlayerState::Ended;
//                    std::unique_lock<std::mutex> lk(m_);
//                    cv_.wait_for(lk, std::chrono::milliseconds(10));
//                }
//            }
//        }
//
//        void deliverVideo(VideoFrame vf) {
//            VideoSink sinkCopy;
//            {
//                std::lock_guard<std::mutex> lk(m_);
//                if (vsink_) sinkCopy = vsink_;
//            }
//            if (sinkCopy) {
//                sinkCopy(vf);
//                return;
//            }
//            std::lock_guard<std::mutex> lk(m_);
//            if (vq_.size() < static_cast<size_t>(cfg_.videoQueue))
//                vq_.push_back(std::move(vf));
//        }
//
//        void deliverAudio(AudioFrame af) {
//            AudioSink sinkCopy;
//            {
//                std::lock_guard<std::mutex> lk(m_);
//                if (asink_) sinkCopy = asink_;
//            }
//            if (sinkCopy) {
//                sinkCopy(af);
//                return;
//            }
//            std::lock_guard<std::mutex> lk(m_);
//            if (aq_.size() < static_cast<size_t>(cfg_.audioQueue))
//                aq_.push_back(std::move(af));
//        }
//
//    private:
//        // Config + demux
//        PlayerConfig                 cfg_{};
//        DemuxInfo                   demux_{};
//        std::unique_ptr<IVideoReader> reader_{};
//
//        // State
//        std::atomic<PlayerState>    state_{ PlayerState::Idle };
//        bool                        haveV_{ false };
//        bool                        haveA_{ false };
//        bool                        loop_{ false };
//        std::atomic<bool>           paused_{ true };
//        std::atomic<bool>           quit_{ false };
//        std::atomic<bool>           eofSeen_{ false };
//        double                      rate_{ 1.0 };
//
//        // Time
//        Clock::time_point           startWall_{};
//        double                      startMedia_{ 0.0 };
//        double                      externalTime_{ 0.0 };
//
//        // Queues
//        mutable std::mutex          m_;
//        std::condition_variable     cv_;
//        std::deque<VideoFrame>      vq_;
//        std::deque<AudioFrame>      aq_;
//
//        // Optional sinks
//        VideoSink                   vsink_{};
//        AudioSink                   asink_{};
//
//        // Thread
//        std::thread                 decodeTh_{};
//
//        std::mutex                  r_;                 // guards reader_ + all FFmpeg calls
//        std::atomic<bool>           pause_io_{ false };   // blocks decode loop during seek/open/close
//        std::atomic<bool>           seeking_{ false };
//    };
//
//    // Unique maker for central factory
//    std::unique_ptr<IPlayer> MakePlayer_FFmpeg() {
//        return std::make_unique<SimplePullPlayer>();
//    }
//
//} // namespace FrameKit::MediaKit
//#endif // FK_MEDIA_ENABLE_FFMPEG