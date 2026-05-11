// retroarch.h: RetroArch launcher module. Discovery side; the
// launcher contract (Claims/Build) registers in retroarch.cpp.

#pragma once

#include <stddef.h>

// Find RetroArch install roots. userOverride from desktop.ini
// [Desktop] RetroArchPath= wins when set. Returns the count.
int RetroArch_DiscoverInstall(const char* userOverride,
                               char outRoots[][512], int maxRoots);

// List core filenames (e.g. "ppsspp_libretro.dll") found in
// <installRoot>/cores/. Returns the count.
int RetroArch_EnumerateCores(const char* installRoot,
                              char outCores[][256], int maxCores);

// Find the directory that holds .lpl playlists. macOS splits this
// from the cores root (Documents/RetroArch/playlists vs Application
// Support/RetroArch/cores). Returns true if a directory was found.
bool RetroArch_DiscoverPlaylistsDir(const char* installRoot,
                                     char* outDir, size_t outSize);

// Resolve a core filename for a given system display name by scanning
// <installRoot>/info/*.info for a matching systemname=, then verifying
// the corresponding dylib/.dll/.so exists in <installRoot>/cores/.
bool RetroArch_FindCoreForSystem(const char* installRoot,
                                  const char* systemName,
                                  char* outCoreFile, size_t outSize);

// Read systemname= from the .info file paired with this core filename.
// coreFilename is "<core>_libretro.<ext>"; we derive the .info path.
bool RetroArch_GetSystemForCore(const char* installRoot,
                                 const char* coreFilename,
                                 char* outSystem, size_t outSize);

// Try to locate the libretro-thumbnails boxart for a playlist item.
bool RetroArch_FindBoxart(const char* installRoot,
                          const char* dbName,
                          const char* label,
                          const char* contentPath,
                          char* outPath, size_t outSize);

typedef void (*RetroArchItemCB)(const char* label,
                                const char* contentPath,
                                const char* dbName,
                                const char* coreName,
                                const char* corePath,
                                void* userdata);

int RetroArch_WalkPlaylists(const char* playlistsDir,
                             RetroArchItemCB cb, void* userdata);
