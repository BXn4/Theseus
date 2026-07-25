// xcloud_video.h: H264 decode for the xCloud stream. Feed Annex-B access
// units from the WebRTC video track; the latest decoded frame is kept as BGRA
// for the UI to upload into a texture.

#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

void XcloudVideo_Start();
void XcloudVideo_Stop();
void XcloudVideo_PushFrame(const uint8_t* annexb, size_t len);

// Copies the latest frame into out if it is newer than *seq (and updates seq).
// Returns true when a new frame was copied. out is BGRA8, w*h*4 bytes.
bool XcloudVideo_GetLatest(std::vector<uint8_t>& out, int& w, int& h, uint64_t& seq);
