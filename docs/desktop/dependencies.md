# Desktop dependencies

What the desktop build links against on each platform, the floor each library
has to clear, and where the Windows cross-deps come from.

## Minimum versions

These are derived from the API calls the code actually makes, not from
whatever happens to be installed on a dev box. Anything at or above these
should link and run.

| Library | Minimum | What pins it |
|---|---|---|
| SDL2 | 2.0.4 | `SDL_QueueAudio` / `SDL_GetQueuedAudioSize` in the xCloud audio path |
| SDL2_mixer | 2.0.0 | base mixer API |
| libavcodec | 57.24 (FFmpeg 3.1) | `avcodec_send_packet` / `avcodec_receive_frame`. No `AVChannelLayout` use, so FFmpeg 5.1 is **not** required |
| libswscale | 4.0 | `sws_getContext` / `sws_scale` |
| opus | 1.0 | `opus_decode`, `opus_decoder_create` / `_destroy`, unchanged since 1.0 |
| libcurl | 7.60 | easy API only |
| libmpv | 1.x | media player node |
| OpenSSL | 1.1.1 | libdatachannel's DTLS |
| Compiler | C++17 | |

## Built and tested against

| | macOS | Linux x64 | Windows x64 |
|---|---|---|---|
| Toolchain | clang 21 (arm64) | gcc 14.2 | mingw-w64 g++ 16.1 |
| SDL2 | 2.32.70 | 2.32.4 | 2.32.10 |
| SDL2_mixer | 2.8.2 | 2.8.1 | 2.8.1 |
| libavcodec | 8.1.2 | 61.19.101 | 61 (7.1.1, static) |
| opus | 1.6.1 | 1.5.2 | 1.6.1 |
| libcurl | 8.21.0 | 8.14.1 | latest release |
| libmpv | 0.41.0 | 2.5.0 | libmpv-2 |
| OpenSSL | 3.6.3 | 3.5.6 | 3.6.3 |
| glibc | n/a | 2.41 | n/a |

The Linux binary links libavcodec 61 and glibc 2.41, and a tester on
libavcodec 62 could not load a build made against 61. That is why the Linux
tarball bundles its whole `ldd` closure except the host-tied libraries
(glibc, GPU driver GL/EGL/Vulkan/DRM, X11/xcb/Wayland/xkb).

## Windows cross-deps

`build/fetch-mingw-deps.sh` populates `~/cross/mingw64` and both CI workflows
call it. Two different strategies in there:

- **OpenSSL and opus** come prebuilt from MSYS2. They are C libraries with no
  onward dependencies, so the gcc that built them does not have to match ours.
- **ffmpeg is built from source**, trimmed to H.264/HEVC decode plus swscale,
  and linked static. MSYS2's ffmpeg is a full codec build whose `avcodec-62.dll`
  pulls in 35 more DLLs (x264, aom, rsvg, glib, ...). We decode one format, so
  a trimmed static build is both smaller and ships nothing extra.

HEVC is enabled even though xCloud sends H.264. `h2645_sei.o` calls
`ff_aom_uninit_film_grain_params`, and libavcodec only compiles
`aom_film_grain.o` when an HEVC config is on, so an h264-only build leaves that
symbol dangling.

The other cross-deps (SDL2, SDL2_mixer, libmpv, libcurl) are fetched by the
workflows into `~/cross/{sdl2-mingw,mpv-mingw,curl-mingw}`.

### Two traps worth knowing

`curl-mingw` ships its own `libssl.a` and `libcrypto.a`, and they are LibreSSL,
which has no `EVP_MAC_*`. Its `-L` lands earlier on the link line, so the
Makefile references OpenSSL by absolute path instead of `-lssl`.

libdatachannel needs `-DRTC_STATIC`. Without it `rtc/common.hpp` marks every
symbol `__declspec(dllimport)` and the link fails on `__imp_` prefixed names
against the static archive. Its libsrtp also needs
`-DENABLE_WARNINGS_AS_ERRORS=OFF`, because `u_long` is 32 bit under LLP64 and
its `%x` format checks trip `-Werror=format`.

## Streaming is opt-in at build time

xCloud and Xbox remote play only compile in when libdatachannel is present:
`build-mac`, `build-linux`, or `build-win64` under
`theseus/third-party/libdatachannel`. Miss that step and the build still goes
green with streaming silently absent, so all three CI jobs assert on the
artifact and the Windows job greps the binary for `rtc::PeerConnection`.
