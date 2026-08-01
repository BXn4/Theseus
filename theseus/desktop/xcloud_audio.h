// xcloud_audio.h: Opus decode + playback for the xCloud stream. Feed the Opus
// payload from each audio RTP frame; it decodes and queues to a dedicated SDL
// audio device (separate from the dashboard mixer).

#pragma once

#include <cstdint>
#include <cstddef>

void XcloudAudio_Start();
void XcloudAudio_Stop();
void XcloudAudio_PushFrame(const uint8_t* opus, size_t len);
