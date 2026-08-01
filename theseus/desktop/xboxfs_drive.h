// Small dependency free leaf, pulled in early by the platform layer.
// Xbox drive letter to runtime folder next to the binary:
//   C: Configs   Q: Data   E: Library
// F: G: R: and the rest have no analog, so DriveToPrefix returns NULL and
// the caller's stat/opendir fails on its own. Also home to the mkdir helpers.

#pragma once

#include <string.h>
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

static inline const char* XboxFS_DriveToPrefix(char drive) {
    if (drive == 'C' || drive == 'c') return "Configs";
    if (drive == 'Q' || drive == 'q') return "Data";
    if (drive == 'E' || drive == 'e') return "Library";
    return 0;
}

// Make one directory. True if it exists after.
static inline bool Plat_Mkdir(const char* path) {
    if (!path || !*path) return false;
#ifdef _WIN32
    return _mkdir(path) == 0 || errno == EEXIST;
#else
    return mkdir(path, 0755) == 0 || errno == EEXIST;
#endif
}

// Make a directory and any missing parents. Accepts either slash style.
static inline bool Plat_MkdirP(const char* path) {
    if (!path || !*path) return false;
    char tmp[1024];
    strncpy(tmp, path, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    for (char* p = tmp; *p; ++p) if (*p == '\\') *p = '/';
    for (char* p = tmp + 1; *p; ++p) {
        if (*p == '/') { *p = '\0'; Plat_Mkdir(tmp); *p = '/'; }
    }
    return Plat_Mkdir(tmp);
}
