#!/bin/bash
# Bundle the desktop build into Theseus.app. Homebrew dylibs get copied into
# Contents/Frameworks and their install names rewritten to @rpath, otherwise
# the app only runs on a machine with your exact brew layout.
set -euo pipefail

SRC="${1:-$HOME/builds/theseus/desktop}"
OUT="${2:-$HOME/builds/theseus/Theseus.app}"
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

rm -rf "$OUT"
mkdir -p "$OUT/Contents/MacOS" "$OUT/Contents/Resources" "$OUT/Contents/Frameworks"

cp "$SRC/theseus" "$OUT/Contents/MacOS/theseus"
cp -R "$REPO/Data/shaders" "$OUT/Contents/Resources/Data_shaders_tmp"
mkdir -p "$OUT/Contents/Resources/Data"
mv "$OUT/Contents/Resources/Data_shaders_tmp" "$OUT/Contents/Resources/Data/shaders"

# Seed payload: everything the user can customize, copied on first run.
for d in Configs Library; do
  [ -d "$REPO/$d" ] && cp -R "$REPO/$d" "$OUT/Contents/Resources/$d"
done
for d in "$REPO"/Data/*/; do
  name="$(basename "$d")"
  [ "$name" = "shaders" ] && continue
  cp -R "$d" "$OUT/Contents/Resources/Data/$name"
done

# Icon: drop a square PNG (1024x1024 ideal) at build/icon.png and it gets
# turned into the full .icns set. Without one the app takes the generic icon.
ICON_SRC="${ICON_SRC:-$REPO/build/icon.png}"
ICON_LINE=""
if [ -f "$ICON_SRC" ]; then
  ICONSET="$(mktemp -d)/theseus.iconset"
  mkdir -p "$ICONSET"
  for sz in 16 32 64 128 256 512; do
    sips -z $sz $sz "$ICON_SRC" --out "$ICONSET/icon_${sz}x${sz}.png" >/dev/null 2>&1
    sips -z $((sz*2)) $((sz*2)) "$ICON_SRC" --out "$ICONSET/icon_${sz}x${sz}@2x.png" >/dev/null 2>&1
  done
  iconutil -c icns "$ICONSET" -o "$OUT/Contents/Resources/theseus.icns" 2>/dev/null \
    || echo "iconutil failed, shipping without an icon"
else
  echo "no icon at $ICON_SRC, shipping without one"
fi

cat > "$OUT/Contents/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleName</key><string>UIX Desktop</string>
  <key>CFBundleDisplayName</key><string>UIX Desktop</string>
  <key>CFBundleExecutable</key><string>theseus</string>
  <key>CFBundleIconFile</key><string>theseus</string>
  <key>CFBundleIdentifier</key><string>com.teamuix.theseus</string>
  <key>CFBundlePackageType</key><string>APPL</string>
  <key>CFBundleShortVersionString</key><string>0.3.3.2</string>
  <key>CFBundleVersion</key><string>0.3.3.2</string>
  <key>LSMinimumSystemVersion</key><string>11.0</string>
  <key>NSHighResolutionCapable</key><true/>
  <key>NSMicrophoneUsageDescription</key><string>Voice chat during cloud game streaming.</string>
</dict>
</plist>
PLIST

BIN="$OUT/Contents/MacOS/theseus"
FW="$OUT/Contents/Frameworks"

# System frameworks and /usr/lib stay put; everything else comes along.
is_system() { case "$1" in /System/*|/usr/lib/*) return 0;; *) return 1;; esac; }

# Homebrew dylibs reference their siblings through @loader_path, so resolve
# those against wherever the dylib came from or they get silently missed.
resolve_dep() {
  local dep="$1" origdir="$2"
  case "$dep" in
    @loader_path/*)     echo "$origdir/${dep#@loader_path/}" ;;
    @executable_path/*) echo "$origdir/${dep#@executable_path/}" ;;
    @rpath/*)           echo "$origdir/${dep#@rpath/}" ;;
    *)                  echo "$dep" ;;
  esac
}

collect() {
  local obj="$1" origdir="$2"
  otool -L "$obj" | tail -n +2 | awk '{print $1}' | while read -r dep; do
    is_system "$dep" && continue
    local real; real="$(resolve_dep "$dep" "$origdir")"
    [ -f "$real" ] || continue
    local base; base="$(basename "$real")"
    if [ ! -f "$FW/$base" ]; then
      cp -L "$real" "$FW/$base" 2>/dev/null || continue
      chmod u+w "$FW/$base"
      collect "$FW/$base" "$(cd "$(dirname "$real")" && pwd)"
    fi
  done
}
collect "$BIN" "$(dirname "$BIN")"

# Homebrew's libSDL2 is sdl2-compat, a shim that dlopens SDL3 at runtime.
# otool never sees it, so pull it in by hand or the app dies at launch with
# "Failed loading SDL3 library".
if [ -f "$FW/libSDL2-2.0.0.dylib" ] && strings "$FW/libSDL2-2.0.0.dylib" 2>/dev/null | grep -q libSDL3; then
  for cand in /opt/homebrew/lib/libSDL3.0.dylib /usr/local/lib/libSDL3.0.dylib \
              "$(brew --prefix 2>/dev/null)/lib/libSDL3.0.dylib"; do
    if [ -f "$cand" ]; then
      cp -L "$cand" "$FW/libSDL3.dylib"
      chmod u+w "$FW/libSDL3.dylib"
      collect "$FW/libSDL3.dylib" "$(cd "$(dirname "$cand")" && pwd)"
      break
    fi
  done
  [ -f "$FW/libSDL3.dylib" ] || echo "WARNING: sdl2-compat needs SDL3 and it was not found"
fi

# projectM ships beside the binary already; fold it in too.
[ -d "$SRC/lib" ] && for l in "$SRC/lib"/*.dylib; do
  [ -f "$l" ] && [ ! -f "$FW/$(basename "$l")" ] && { cp -L "$l" "$FW/"; chmod u+w "$FW/$(basename "$l")"; }
done

retarget() {
  local obj="$1"
  otool -L "$obj" | tail -n +2 | awk '{print $1}' | while read -r dep; do
    is_system "$dep" && continue
    local base; base="$(basename "$dep")"
    [ -f "$FW/$base" ] && install_name_tool -change "$dep" "@rpath/$base" "$obj" 2>/dev/null || true
  done
}
for f in "$FW"/*.dylib; do
  [ -f "$f" ] || continue
  install_name_tool -id "@rpath/$(basename "$f")" "$f" 2>/dev/null || true
  retarget "$f"
done
retarget "$BIN"
install_name_tool -add_rpath "@executable_path/../Frameworks" "$BIN" 2>/dev/null || true

# Ad-hoc signature so Gatekeeper lets it run locally. A Developer ID and
# notarization are still needed before handing it to anyone else.
codesign --force --deep --sign - "$OUT" >/dev/null 2>&1 || echo "codesign failed (not fatal for local runs)"

echo "bundle: $OUT"
echo "size:   $(du -sh "$OUT" | cut -f1)"
echo "frameworks: $(ls "$FW" | wc -l | tr -d ' ')"
