#!/usr/bin/env bash
# Fetch the mingw-w64 dev libraries the Windows build needs that neither
# Homebrew nor apt ship cross-compiled: OpenSSL (for libdatachannel), opus,
# and ffmpeg (libavcodec/libavutil/libswscale for xCloud video decode).
#
# MSYS2 publishes these prebuilt for x86_64-w64-mingw32 over plain HTTPS, so
# this works the same on a mac and on a CI runner. They are C libraries, so
# the gcc that built them does not have to match ours.
#
# Lands a sysroot-shaped prefix:
#   $PREFIX/include  $PREFIX/lib (.dll.a import libs)  $PREFIX/bin (.dll)
#
# Usage: build/fetch-mingw-deps.sh [prefix]     (default ~/cross/mingw64)

set -euo pipefail

PREFIX="${1:-$HOME/cross/mingw64}"
BASE=https://repo.msys2.org/mingw/mingw64

# Pinned so a CI run and a local run produce the same binary. Bump
# deliberately; see docs for the minimum versions the code actually needs.
#
# ffmpeg is NOT taken from MSYS2. That package is a full codec build whose
# avcodec pulls in 35 more DLLs (x264, aom, rsvg, glib, ...). We decode H.264
# and scale, nothing else, so a trimmed static build is smaller and ships no
# runtime DLL at all. 7.1.1 is libavcodec 61, the same as the Linux target.
PKGS=(
    mingw-w64-x86_64-openssl-3.6.3-1-any.pkg.tar.zst
    mingw-w64-x86_64-opus-1.6.1-1-any.pkg.tar.zst
)
FFMPEG_VER=7.1.1

command -v zstd >/dev/null || { echo "need zstd (brew install zstd / apt install zstd)" >&2; exit 1; }

mkdir -p "$PREFIX"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

for p in "${PKGS[@]}"; do
    echo "  fetching $p"
    curl -fsSL -o "$TMP/$p" "$BASE/$p"
    # Packages lay out under mingw64/, strip it so we get a clean sysroot.
    tar --use-compress-program=unzstd -xf "$TMP/$p" -C "$TMP"
done

mkdir -p "$PREFIX/include" "$PREFIX/lib" "$PREFIX/bin"
cp -R "$TMP/mingw64/include/." "$PREFIX/include/"
cp -R "$TMP/mingw64/lib/."     "$PREFIX/lib/"
cp -R "$TMP/mingw64/bin/."     "$PREFIX/bin/"

# Trimmed static ffmpeg: H.264/HEVC decode + swscale. --disable-autodetect
# keeps configure from linking anything off the build host. HEVC is not
# optional decoration: h2645_sei.o calls ff_aom_uninit_film_grain_params, and
# libavcodec only compiles aom_film_grain.o when an HEVC config is on, so an
# h264-only build leaves that symbol dangling at link time.
if [ ! -f "$PREFIX/lib/libavcodec.a" ]; then
    echo "  building ffmpeg $FFMPEG_VER (h264 + swscale only)"
    command -v nasm >/dev/null || { echo "need nasm (brew install nasm / apt install nasm)" >&2; exit 1; }
    curl -fsSL -o "$TMP/ff.tar.xz" "https://ffmpeg.org/releases/ffmpeg-$FFMPEG_VER.tar.xz"
    tar -xf "$TMP/ff.tar.xz" -C "$TMP"
    (
        cd "$TMP/ffmpeg-$FFMPEG_VER"
        ./configure \
            --prefix="$PREFIX" \
            --arch=x86_64 --target-os=mingw32 \
            --cross-prefix=x86_64-w64-mingw32- \
            --enable-static --disable-shared --enable-pic \
            --disable-autodetect --disable-programs --disable-doc \
            --disable-network --disable-avdevice --disable-avformat \
            --disable-avfilter --disable-swresample --disable-postproc \
            --disable-everything \
            --enable-decoder=h264 --enable-parser=h264 \
            --enable-decoder=hevc --enable-parser=hevc \
            --enable-swscale \
            >/dev/null
        make -j"$(getconf _NPROCESSORS_ONLN)" >/dev/null
        make install >/dev/null
    )
fi

echo "mingw deps -> $PREFIX"
for l in libssl libcrypto libopus; do
    printf "  %-12s %s\n" "$l" "$(ls "$PREFIX/lib/$l"*.dll.a 2>/dev/null | head -1 || echo MISSING)"
done
for l in libavcodec libavutil libswscale; do
    printf "  %-12s %s\n" "$l" "$([ -f "$PREFIX/lib/$l.a" ] && echo "$PREFIX/lib/$l.a (static)" || echo MISSING)"
done
