// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/MediaKit/MediaKit.h
// Author       : George Gil
// Created      : 2025-09-19
// Updated      : 2025-09-19
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//		Central include for media system. Includes media backends
//		and provides utility functions to access media resources.
// =============================================================================

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <functional>

namespace FrameKit::MediaKit {
    // -------------------- Formats --------------------
    enum class PixelFormat { RGBA8, BGRA8, R16F, R32F, YUV420P, NV12, P010 };
    enum class ColorPrimaries { BT709, BT2020 };
    enum class TransferFunc { SRGB, PQ, HLG };

    struct ColorSpace {
        ColorPrimaries prim{ ColorPrimaries::BT709 };
        TransferFunc   tf{ TransferFunc::SRGB };
        bool           fullRange{ true };
    };

    // -------------------- Images --------------------
    struct ImageDesc {
        int         w{ 0 }, h{ 0 };
        PixelFormat fmt{ PixelFormat::RGBA8 };
        ColorSpace  cs{};
    };

    struct ImageData {
        ImageDesc              desc{};
        std::vector<uint8_t>   owned;                 // CPU storage
        std::span<const uint8_t> bytes() const { return owned; }
    };

    // -------------------- AV Streams ----------------
    struct VideoStreamInfo {
        int         w{ 0 }, h{ 0 };
        double      fps{ 0.0 };
        PixelFormat decodeFmt{ PixelFormat::NV12 };
        ColorSpace  cs{};
    };
    struct AudioStreamInfo {
        int      sampleRate{ 0 };
        int      channels{ 0 };
        int64_t  channelMask{ 0 };
    };

    struct FramePTS { int64_t num{ 0 }, den{ 1 }; };    // num/den seconds

    struct VideoFrame {
        VideoStreamInfo info{};
        FramePTS        pts{};                       // presentation timestamp
        // Planar or packed CPU memory. For NV12/YUV420P planes.size() >= 2. For RGBA8 planes.size()==1.
        std::vector<std::vector<uint8_t>> planes;
        // GPU handle could be added later via variant/opaque pointer.
    };

    struct AudioFrame {
        AudioStreamInfo info{};
        FramePTS        pts{};                       // presentation timestamp
        std::vector<uint8_t> pcmF32;                 // interleaved float32
    };

    struct DemuxInfo {
        std::optional<VideoStreamInfo> video;
        std::optional<AudioStreamInfo> audio;
        double durationSec{ 0.0 };
        bool   isSeekable{ false };
    };

    // -------------------- Interfaces ----------------
    struct IImageLoader {
        virtual ~IImageLoader() = default;
        virtual ImageData load(std::string_view path) = 0;
    };

    // IVideoReader: packet->frame decoder. Stateless w.r.t. scheduling.
    struct IVideoReader {
        virtual ~IVideoReader() = default;

        // Open returns stream layout and container facts.
        virtual DemuxInfo open(std::string_view path) = 0;

        // Decode next frame of requested types.
        // If vf!=nullptr, may return a decoded video frame.
        // If af!=nullptr, may return a decoded audio frame.
        // Returns true iff it produced a frame of a requested type.
        // Returns false on EOF (after flushing) or fatal error.
        virtual bool read(VideoFrame* vf, AudioFrame* af) = 0;

        // Seek to seconds. `exact=false` may land on keyframe.
        virtual bool seek(double seconds, bool exact) = 0;

        virtual void close() = 0;
    };

    // Runtime/player API
    enum class PlayerState { Idle, Opening, Paused, Playing, Stopped, Ended, Error };

    // Demux + static properties
    struct MediaInfo {
        DemuxInfo demux{};
    };

    enum class ClockMode { Auto /*audio if present*/, Video, External };

    struct PlayerConfig {
        bool        hwDecode{ false };
        PixelFormat outFmt{ PixelFormat::RGBA8 };
        bool        deliverGPU{ false };

        // Basic buffering
        int         videoQueue{ 8 };
        int         audioQueue{ 32 };

        // Clocking
        ClockMode   clockMode{ ClockMode::Auto };
        double      maxAVDesync{ 0.100 }; // seconds
    };

    using VideoSink = std::function<void(const VideoFrame&)>;
    using AudioSink = std::function<void(const AudioFrame&)>;

    struct IPlayer {
        virtual ~IPlayer() = default;

        // Lifecycle
        virtual bool open(std::string_view path, const PlayerConfig& cfg) = 0;
        virtual void close() = 0;

        // Control
        virtual void play() = 0;
        virtual void pause() = 0;
        virtual void stop() = 0;
        virtual bool seek(double s, bool exact = false) = 0;
        virtual bool setRate(double rate) = 0;     // optional 0.5..2.0, may be stubbed
        virtual void setLoop(bool loop) = 0;

        // Query
        virtual PlayerState state() const = 0;
        virtual MediaInfo   info()  const = 0;
        virtual double      time()  const = 0;     // current media time in seconds

        // Pull model: non-blocking; true iff a frame is ready to present now.
        virtual bool getVideo(VideoFrame& out) = 0;
        virtual bool getAudio(AudioFrame& out) = 0;

        // Optional push model
        virtual void setVideoSink(VideoSink s) = 0;
        virtual void setAudioSink(AudioSink s) = 0;

        // External clock (when ClockMode::External)
        virtual void setExternalTime(double tSeconds) = 0;
    };

    // -------------------- Factory -------------------
    enum class ImageBackend { Stb, OIIO };
    enum class VideoBackend { FFmpeg /*, GStreamer, MediaFoundation*/ };
    enum class PlayerBackend { FFmpeg /*, Platform*/ };

    std::unique_ptr<IImageLoader> CreateImageLoader(ImageBackend b);
    std::unique_ptr<IVideoReader> CreateVideoReader(VideoBackend b);
    std::unique_ptr<IPlayer>      CreatePlayer(PlayerBackend b);

    // -------------------- Helpers -------------------
    inline double PtsSeconds(const FramePTS& p) {
        return p.den != 0 ? static_cast<double>(p.num) / static_cast<double>(p.den) : 0.0;
    }

} // namespace FrameKit::MediaKit
