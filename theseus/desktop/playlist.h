#pragma once

// User-authored playlists of media items. Stored as paths so they survive
// metadata rescans. Persisted to Configs/playlists.ini.

#include <string>
#include <vector>

struct PlaylistItem {
    std::string path;     // absolute path to video file
    std::string title;    // cached display title (looked up at add time)
};

struct Playlist {
    std::string               name;
    std::vector<PlaylistItem> items;
};

// Lifecycle
void Playlist_LoadAll();
void Playlist_SaveAll();   // explicit save; clears dirty
bool Playlist_IsDirty();

// Enumeration
int                Playlist_Count();
const Playlist*    Playlist_Get(int idx);
Playlist*          Playlist_Find(const char* name);

// CRUD
bool Playlist_Create(const char* name);
bool Playlist_Delete(const char* name);
bool Playlist_Rename(const char* oldName, const char* newName);

// Item ops. idx -1 appends.
bool Playlist_AddItem(const char* name, const char* path, const char* title);
bool Playlist_RemoveItem(const char* name, int idx);
bool Playlist_MoveItem(const char* name, int from, int to);
