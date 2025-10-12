// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/MediaKit/FFmpeg/FFCommon.h
// Author       : George Gil
// Created      : 2025-09-19
// Updated      : 2025-09-19
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//		Common FFmpeg-related utilities.
// =============================================================================
#pragma once
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/pixdesc.h"
#include "libavutil/channel_layout.h"
}

#include "FrameKit/MediaKit/MediaKit.h"

namespace FrameKit::MediaKit::FF {
    inline PixelFormat MapPixFmt(AVPixelFormat f) {
        switch (f) {
        case AV_PIX_FMT_NV12:     return PixelFormat::NV12;
        case AV_PIX_FMT_YUV420P:  return PixelFormat::YUV420P;
        case AV_PIX_FMT_RGBA:     return PixelFormat::RGBA8;
        case AV_PIX_FMT_BGRA:     return PixelFormat::BGRA8;
        case AV_PIX_FMT_P010:     return PixelFormat::P010;
        default:                  return PixelFormat::RGBA8; // fallback after convert
        }
    }

    inline ColorPrimaries MapPrimaries(AVColorPrimaries p) {
        return (p == AVCOL_PRI_BT2020) ? ColorPrimaries::BT2020 : ColorPrimaries::BT709;
    }

    inline TransferFunc MapTRC(AVColorTransferCharacteristic t) {
        if (t == AVCOL_TRC_SMPTE2084) return TransferFunc::PQ;
        if (t == AVCOL_TRC_ARIB_STD_B67) return TransferFunc::HLG;
        return TransferFunc::SRGB;
    }

    inline FramePTS ToPTS(int64_t ts, AVRational tb) {
        FramePTS p; p.num = ts * tb.num; p.den = tb.den; return p;
    }
}
