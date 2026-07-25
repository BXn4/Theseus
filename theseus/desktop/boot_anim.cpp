// boot_anim.cpp: cold-boot animation playback. Brings up a fresh libmpv
// instance, points it at the boot video, renders fullscreen until the
// file ends or the user skips with Esc/Enter/Space. On exit the mpv
// context is torn down completely so the dashboard's own libmpv instance
// (the CDVDPlayer node) can initialize cleanly afterward.
//
// The bundled video (Configs/xbox_boot.mp4) is a 1080p60
// capture of the original Xbox 2001 boot animation. Source:
//   https://www.youtube.com/watch?v=oADANrDGhoQ
// Original boot animation (c) Microsoft / Pipeworks Software 2001.

#include "boot_anim.h"

#include <SDL.h>

#include <bgfx/bgfx.h>
#include "d3d8_sdl.h"   // for g_bgfxProgBlit, g_bgfxSamplerBlit

#include <mpv/client.h>
#include <mpv/render.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>

// Boot anim: libmpv software-renders into a CPU buffer, we upload that to
// a bgfx texture each frame and draw it as a fullscreen quad (vs_blit/fs_blit).
bool BootAnim_PlayAndWait(SDL_Window* win, const char* path)
{
	if (!win || !path || !*path) return false;

	struct stat st;
	if (stat(path, &st) != 0) {
		fprintf(stderr, "[boot_anim] file not found: %s\n", path);
		return false;
	}

	if (!bgfx::isValid(g_bgfxProgBlit)) {
		fprintf(stderr, "[boot_anim] blit program not initialized; skipping\n");
		return false;
	}

	mpv_handle* mpv = mpv_create();
	if (!mpv) return false;

	mpv_set_option_string(mpv, "vo",         "libmpv");
	mpv_set_option_string(mpv, "hwdec",      "no");
	mpv_set_option_string(mpv, "keep-open",  "no");
	mpv_set_option_string(mpv, "video",      "yes");
	mpv_set_option_string(mpv, "audio",      "yes");
	mpv_set_option_string(mpv, "ao",         "coreaudio,wasapi,pulse,sdl");
	mpv_set_option_string(mpv, "volume",     "100");
	mpv_set_option_string(mpv, "mute",       "no");
	mpv_set_option_string(mpv, "osc",        "no");
	mpv_set_option_string(mpv, "terminal",   "no");
	mpv_set_option_string(mpv, "msg-level",  "all=error");

	if (mpv_initialize(mpv) < 0) {
		mpv_destroy(mpv);
		return false;
	}

	int apiSw = (int)MPV_RENDER_API_TYPE_SW;
	(void)apiSw;
	mpv_render_param createParams[] = {
		{ MPV_RENDER_PARAM_API_TYPE, (void*)MPV_RENDER_API_TYPE_SW },
		{ MPV_RENDER_PARAM_INVALID,  nullptr }
	};

	mpv_render_context* rc = nullptr;
	if (mpv_render_context_create(&rc, mpv, createParams) < 0) {
		mpv_destroy(mpv);
		return false;
	}

	const char* loadCmd[] = { "loadfile", path, nullptr };
	if (mpv_command(mpv, loadCmd) < 0) {
		mpv_render_context_free(rc);
		mpv_destroy(mpv);
		return false;
	}

	// Source resolution is fixed (1080p capture); SW render targets a
	// CPU buffer of this size and the GPU upsamples via linear filter
	// when we draw the fullscreen quad.
	const int kW = 1920;
	const int kH = 1080;
	const size_t kStride = (size_t)kW * 4;
	const size_t kBufSize = kStride * (size_t)kH;
	uint8_t* swBuf = (uint8_t*)std::calloc(1, kBufSize);
	if (!swBuf) {
		mpv_render_context_free(rc);
		mpv_terminate_destroy(mpv);
		return false;
	}

	bgfx::TextureHandle tex = bgfx::createTexture2D(
		(uint16_t)kW, (uint16_t)kH, false, 1,
		bgfx::TextureFormat::BGRA8,
		BGFX_TEXTURE_NONE | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);

	bool running    = true;
	bool sawAnyDraw = false;

	while (running) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) running = false;
			else if (e.type == SDL_KEYDOWN &&
			         (e.key.keysym.sym == SDLK_ESCAPE ||
			          e.key.keysym.sym == SDLK_RETURN ||
			          e.key.keysym.sym == SDLK_SPACE))
				running = false;
		}

		while (true) {
			mpv_event* ev = mpv_wait_event(mpv, 0.0);
			if (!ev || ev->event_id == MPV_EVENT_NONE) break;
			if (ev->event_id == MPV_EVENT_END_FILE) { running = false; break; }
		}
		if (!running) break;

		uint64_t flags = mpv_render_context_update(rc);
		if (flags & MPV_RENDER_UPDATE_FRAME) {
			int swSize[2]  = { kW, kH };
			char swFmt[]   = "bgra"; // libmpv writes B,G,R,A bytes in
			                         // memory order; bgfx BGRA8 reads
			                         // those bytes as R=B G=G B=R A=A
			                         // (it's a misnomer. BGRA8 means
			                         // "bytes B,G,R,A" in bgfx land).
			size_t swStride = kStride;
			mpv_render_param drawParams[] = {
				{ MPV_RENDER_PARAM_SW_SIZE,    swSize },
				{ MPV_RENDER_PARAM_SW_FORMAT,  swFmt },
				{ MPV_RENDER_PARAM_SW_STRIDE,  &swStride },
				{ MPV_RENDER_PARAM_SW_POINTER, swBuf },
				{ MPV_RENDER_PARAM_INVALID,    nullptr }
			};
			if (mpv_render_context_render(rc, drawParams) == 0) {
				bgfx::updateTexture2D(tex, 0, 0, 0, 0,
					(uint16_t)kW, (uint16_t)kH,
					bgfx::copy(swBuf, (uint32_t)kBufSize));
				sawAnyDraw = true;
			}
		}

		// Adjust view 0 to the current window pixel size each frame
		// in case the user resized; cheap to set even when unchanged.
		int winW = 0, winH = 0;
		SDL_GetWindowSize(win, &winW, &winH);
		if (winW <= 0) winW = 1;
		if (winH <= 0) winH = 1;
		bgfx::setViewRect(0, 0, 0, (uint16_t)winW, (uint16_t)winH);
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR, 0x000000ff, 1.0f, 0);

		Bgfx_BlitTexturedQuad(tex, 1.f, 1.f, 0);

		bgfx::frame();
	}

	bgfx::destroy(tex);
	mpv_render_context_free(rc);
	mpv_terminate_destroy(mpv);
	std::free(swBuf);
	return sawAnyDraw;
}
