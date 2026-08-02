// Virtual game library. Serves game entries straight from games.ini with no
// real folders, so the dashboard sees virtual folders, XBEs, and icons.
// Desktop only.

#pragma once

#include "app_paths.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include "xboxfs_drive.h"

// Roomy enough for a full local library plus a signed-in xCloud/Game Pass
// account injected at boot (see xcloud_vgames.cpp).
#define VGAMES_MAX 2048
#define VGAMES_INI AppPath("Configs/games.ini")
#define VGAMES_ICONS AppPath_Tree("Configs/icons")

struct VirtualGame {
    char name[128];        // display name and virtual folder name
    char titleID[16];      // hex title id, e.g. "45410013"
    char launch[512];
    char drive[4];         // "E", "F", "G"
    char category[32];     // "Games", "Applications", "Homebrew", ...
    bool valid;
};

struct VirtualGameDB {
    VirtualGame games[VGAMES_MAX];
    int count;
    bool loaded;
    // Bumps on every mutation so consumers can cache filtered slices keyed on
    // it and auto refresh when games.ini changes.
    unsigned int generation;
};

extern VirtualGameDB g_vgames;

// A live streaming entry (xCloud / remote play), injected at boot from a
// signed-in account and never written to games.ini. Identified by its launch
// scheme so no extra field is needed.
inline bool VGames_IsStreaming(const VirtualGame& g) {
    return strncmp(g.launch, "xcloud://", 9) == 0 || strncmp(g.launch, "xhome://", 8) == 0;
}

inline void VGames_Load() {
    if (g_vgames.loaded) return;
    g_vgames.loaded = true;
    g_vgames.count = 0;

    FILE* fp = fopen(VGAMES_INI, "r");
    if (!fp) return;

    char line[1024];
    VirtualGame* cur = NULL;

    while (fgets(line, sizeof(line), fp)) {
        char* nl = strchr(line, '\n'); if (nl) *nl = 0;
        char* cr = strchr(line, '\r'); if (cr) *cr = 0;
        if (line[0] == 0) continue;

        // strrchr so ROM tags like [!] inside the name survive.
        if (line[0] == '[') {
            char* end = strrchr(line, ']');
            if (end && g_vgames.count < VGAMES_MAX) {
                cur = &g_vgames.games[g_vgames.count];
                memset(cur, 0, sizeof(*cur));
                *end = 0;
                strncpy(cur->name, line + 1, sizeof(cur->name) - 1);
                strcpy(cur->drive, "E");
                strcpy(cur->category, "Games");
                cur->valid = true;
                g_vgames.count++;
            }
            continue;
        }

        if (!cur) continue;

        char* eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        const char* key = line;
        const char* val = eq + 1;

        if (strcmp(key, "TitleID") == 0)
            strncpy(cur->titleID, val, sizeof(cur->titleID) - 1);
        else if (strcmp(key, "Launch") == 0)
            strncpy(cur->launch, val, sizeof(cur->launch) - 1);
        else if (strcmp(key, "Drive") == 0)
            strncpy(cur->drive, val, sizeof(cur->drive) - 1);
        else if (strcmp(key, "Category") == 0)
            strncpy(cur->category, val, sizeof(cur->category) - 1);
    }
    fclose(fp);
    g_vgames.generation++;
    fprintf(stdout, "[VGames] Loaded %d virtual games from games.ini (gen %u)\n",
            g_vgames.count, g_vgames.generation);
}

inline void VGames_Reload() {
    g_vgames.loaded = false;
    g_vgames.count = 0;
    VGames_Load();
}

inline void VGames_Save() {
    Plat_MkdirP("Configs");

    FILE* fp = fopen(VGAMES_INI, "w");
    if (!fp) return;

    for (int i = 0; i < g_vgames.count; i++) {
        VirtualGame& g = g_vgames.games[i];
        if (!g.valid) continue;
        if (VGames_IsStreaming(g)) continue;   // xCloud/remote-play entries are live, never persisted
        fprintf(fp, "[%s]\n", g.name);
        fprintf(fp, "TitleID=%s\n", g.titleID);
        fprintf(fp, "Launch=%s\n", g.launch);
        fprintf(fp, "Drive=%s\n", g.drive);
        fprintf(fp, "Category=%s\n", g.category);
        fprintf(fp, "\n");
    }
    fclose(fp);
}

inline int VGames_Add(const char* name, const char* titleID, const char* launch,
                       const char* drive, const char* category) {
    VGames_Load();
    if (g_vgames.count >= VGAMES_MAX) return -1;

    VirtualGame& g = g_vgames.games[g_vgames.count];
    memset(&g, 0, sizeof(g));
    strncpy(g.name, name, sizeof(g.name) - 1);
    strncpy(g.titleID, titleID, sizeof(g.titleID) - 1);
    strncpy(g.launch, launch, sizeof(g.launch) - 1);
    strncpy(g.drive, drive ? drive : "E", sizeof(g.drive) - 1);
    strncpy(g.category, category ? category : "Games", sizeof(g.category) - 1);
    g.valid = true;
    g_vgames.generation++;
    return g_vgames.count++;
}

inline void VGames_Update(int idx, const char* name, const char* titleID,
                           const char* launch, const char* drive, const char* category) {
    if (idx < 0 || idx >= g_vgames.count) return;
    VirtualGame& g = g_vgames.games[idx];
    if (name && name[0])     strncpy(g.name, name, sizeof(g.name) - 1);
    if (titleID && titleID[0])  strncpy(g.titleID, titleID, sizeof(g.titleID) - 1);
    if (launch)   strncpy(g.launch, launch, sizeof(g.launch) - 1);
    if (drive && drive[0])    strncpy(g.drive, drive, sizeof(g.drive) - 1);
    if (category && category[0]) strncpy(g.category, category, sizeof(g.category) - 1);
    g_vgames.generation++;
}

inline int VGames_FindByName(const char* name) {
    VGames_Load();
    for (int i = 0; i < g_vgames.count; i++) {
        if (g_vgames.games[i].valid && strcasecmp(g_vgames.games[i].name, name) == 0)
            return i;
    }
    return -1;
}

inline void VGames_DeleteByName(const char* name) {
    VGames_Load();
    int idx = VGames_FindByName(name);
    if (idx < 0) return;

    char iconPath[512];
    snprintf(iconPath, sizeof(iconPath), "%s", AppPathf("Configs/icons/%s.jpg", g_vgames.games[idx].titleID));
    remove(iconPath);

    g_vgames.games[idx].valid = false;
    g_vgames.generation++;

    FILE* fp = fopen(AppPath("Configs/games.ini"), "w");
    if (fp) {
        for (int i = 0; i < g_vgames.count; i++) {
            if (!g_vgames.games[i].valid) continue;
            if (VGames_IsStreaming(g_vgames.games[i])) continue;
            fprintf(fp, "[%s]\n", g_vgames.games[i].name);
            fprintf(fp, "TitleID=%s\n", g_vgames.games[i].titleID);
            fprintf(fp, "Launch=%s\n", g_vgames.games[i].launch);
            fprintf(fp, "Drive=%s\n", g_vgames.games[i].drive);
            if (g_vgames.games[i].category[0])
                fprintf(fp, "Category=%s\n", g_vgames.games[i].category);
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
    fprintf(stderr, "[VGames] Deleted: %s\n", name);
}

// Match a disk path like "Library/Games/Road Rage" to a game index, or -1.
inline int VGames_MatchFolder(const char* localPath) {
    VGames_Load();
    char norm[512];
    int j = 0;
    for (int k = 0; localPath[k] && j < 510; k++) {
        if (localPath[k] == '/' && j > 0 && norm[j-1] == '/') continue;
        norm[j++] = localPath[k];
    }
    if (j > 0 && norm[j-1] == '/') j--;
    norm[j] = 0;

    for (int i = 0; i < g_vgames.count; i++) {
        if (!g_vgames.games[i].valid) continue;
        // Games carry an Xbox drive letter but live under prefix/category/name
        // on desktop. F/G/R never reach here since they have no analog.
        const char* prefix = (g_vgames.games[i].drive[0])
            ? XboxFS_DriveToPrefix(g_vgames.games[i].drive[0]) : 0;
        if (!prefix) continue;
        char expected[512];
        snprintf(expected, sizeof(expected), "%s/%s/%s",
                 prefix, g_vgames.games[i].category, g_vgames.games[i].name);
        if (strcasecmp(norm, expected) == 0)
            return i;
    }
    return -1;
}

// Split a dir like "Library/Games" into drive + category. Used by both
// FindFirstFile shims.
inline bool VGames_ParseDir(const char* dirPath, char* outDrive, char* outCat, size_t catLen) {
    if (!dirPath || !outDrive || !outCat || catLen < 2) return false;
    static const struct { const char* prefix; char drive; } kMap[] = {
        { "Library", 'E' }, { "Data", 'Q' }, { "Configs", 'C' }
    };
    const char* p = NULL;
    outDrive[0] = 0;
    for (size_t i = 0; i < sizeof(kMap) / sizeof(kMap[0]); i++) {
        const char* hit = strstr(dirPath, kMap[i].prefix);
        if (!hit) continue;
        size_t plen = strlen(kMap[i].prefix);
        if (hit[plen] != '\\' && hit[plen] != '/') continue;
        outDrive[0] = kMap[i].drive;
        outDrive[1] = 0;
        p = hit + plen + 1;
        break;
    }
    if (!p) return false;

    size_t ci = 0;
    while (*p && *p != '\\' && *p != '/' && ci < catLen - 1)
        outCat[ci++] = *p++;
    outCat[ci] = 0;

    static const char* gameCats[] = { "Games", "Applications", "Apps", "Homebrew", "Emulators", "Dashboards" };
    for (size_t i = 0; i < sizeof(gameCats) / sizeof(gameCats[0]); i++) {
        if (strcasecmp(outCat, gameCats[i]) == 0) return true;
    }
    return false;
}

inline int VGames_GetForDirectory(const char* drive, const char* category,
                                   int* outIndices, int maxOut) {
    VGames_Load();
    int count = 0;
    for (int i = 0; i < g_vgames.count && count < maxOut; i++) {
        if (!g_vgames.games[i].valid) continue;
        if (strcasecmp(g_vgames.games[i].drive, drive) == 0 &&
            strcasecmp(g_vgames.games[i].category, category) == 0) {
            outIndices[count++] = i;
        }
    }
    return count;
}

inline const char* VGames_GetIconPath(int idx) {
    static char s_iconBuf[512];
    if (idx < 0 || idx >= g_vgames.count) return NULL;
    snprintf(s_iconBuf, sizeof(s_iconBuf), "%s/%s.jpg", VGAMES_ICONS, g_vgames.games[idx].titleID);
    struct stat st;
    if (stat(s_iconBuf, &st) == 0)
        return s_iconBuf;
    snprintf(s_iconBuf, sizeof(s_iconBuf), "%s/%s.png", VGAMES_ICONS, g_vgames.games[idx].titleID);
    if (stat(s_iconBuf, &st) == 0)
        return s_iconBuf;
    return NULL;
}

inline int VGames_MakeShortcutContent(int idx, char* buf, int bufSize) {
    if (idx < 0 || idx >= g_vgames.count) return 0;
    VirtualGame& g = g_vgames.games[idx];
    return snprintf(buf, bufSize,
        "[Title]\nName=%s\nTitleID=%s\nLaunch=%s\n",
        g.name, g.titleID, g.launch);
}
