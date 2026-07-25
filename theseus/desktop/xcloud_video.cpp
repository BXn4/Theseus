// xcloud_video.cpp: H264 to BGRA using libavcodec + libswscale. The WebRTC
// track hands us complete Annex-B access units (SPS/PPS/IDR/P in band), so we
// feed each straight to the decoder and colour convert whatever comes out.
//
// Gated on WebRTC: with no libdatachannel there is no xCloud, so the ffmpeg
// dependency (and this whole file) drops out. Matches xcloud_client.cpp.

#if defined(THESEUS_HAVE_WEBRTC)

#include "xcloud_video.h"

#include <mutex>
#include <cstdio>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

static std::mutex        s_mtx;
static AVCodecContext*   s_ctx   = nullptr;
static AVPacket*         s_pkt   = nullptr;
static AVFrame*          s_frame = nullptr;
static SwsContext*       s_sws   = nullptr;
static int               s_swsW = 0, s_swsH = 0;

static std::vector<uint8_t> s_latest;   // BGRA
static int      s_w = 0, s_h = 0;
static uint64_t s_seq = 0;

void XcloudVideo_Start()
{
    std::lock_guard<std::mutex> l(s_mtx);
    if (s_ctx) return;
    const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) { fprintf(stderr, "[xCloudVideo] no H264 decoder\n"); return; }
    s_ctx = avcodec_alloc_context3(codec);
    if (!s_ctx || avcodec_open2(s_ctx, codec, nullptr) < 0) {
        fprintf(stderr, "[xCloudVideo] decoder open failed\n");
        if (s_ctx) avcodec_free_context(&s_ctx);
        s_ctx = nullptr; return;
    }
    s_pkt   = av_packet_alloc();
    s_frame = av_frame_alloc();
    s_seq = 0;
    fprintf(stderr, "[xCloudVideo] decoder ready\n");
}

void XcloudVideo_Stop()
{
    std::lock_guard<std::mutex> l(s_mtx);
    if (s_sws)   { sws_freeContext(s_sws); s_sws = nullptr; s_swsW = s_swsH = 0; }
    if (s_frame) av_frame_free(&s_frame);
    if (s_pkt)   av_packet_free(&s_pkt);
    if (s_ctx)   avcodec_free_context(&s_ctx);
    s_ctx = nullptr;
    s_latest.clear(); s_w = s_h = 0; s_seq = 0;
}

void XcloudVideo_PushFrame(const uint8_t* annexb, size_t len)
{
    std::lock_guard<std::mutex> l(s_mtx);
    if (!s_ctx || !len) return;

    s_pkt->data = const_cast<uint8_t*>(annexb);
    s_pkt->size = (int)len;
    if (avcodec_send_packet(s_ctx, s_pkt) < 0) return;

    while (avcodec_receive_frame(s_ctx, s_frame) == 0) {
        int w = s_frame->width, h = s_frame->height;
        if (w <= 0 || h <= 0) continue;
        if (!s_sws || s_swsW != w || s_swsH != h) {
            if (s_sws) sws_freeContext(s_sws);
            s_sws = sws_getContext(w, h, (AVPixelFormat)s_frame->format,
                                   w, h, AV_PIX_FMT_BGRA, SWS_BILINEAR,
                                   nullptr, nullptr, nullptr);
            s_swsW = w; s_swsH = h;
        }
        if (!s_sws) continue;
        s_latest.resize((size_t)w * h * 4);
        uint8_t* dst[4]     = { s_latest.data(), nullptr, nullptr, nullptr };
        int      dstLine[4] = { w * 4, 0, 0, 0 };
        sws_scale(s_sws, s_frame->data, s_frame->linesize, 0, h, dst, dstLine);
        s_w = w; s_h = h; s_seq++;
    }
}

bool XcloudVideo_GetLatest(std::vector<uint8_t>& out, int& w, int& h, uint64_t& seq)
{
    std::lock_guard<std::mutex> l(s_mtx);
    if (s_seq == 0 || s_seq == seq || s_latest.empty()) return false;
    out = s_latest; w = s_w; h = s_h; seq = s_seq;
    return true;
}

#endif // THESEUS_HAVE_WEBRTC
