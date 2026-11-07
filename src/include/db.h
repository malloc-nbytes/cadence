#ifndef DB_H_INCLUDED
#define DB_H_INCLUDED

#include "sqlite3.h"
#include "array.h"
#include "str.h"

#include <stdint.h>

typedef struct {
        sqlite3_int64 id;

        sqlite3_int64 artist_id;
        sqlite3_int64 album_id;

        str artist;
        str album;
        str name;
        str path;

        int64_t duration_ms;
        int64_t file_mtime;
        int64_t file_size;
        int64_t added_at;
        int64_t last_played_at;

        unsigned track_number;
        unsigned disc_number;
        unsigned play_count;
} track;

ARRAY_DEFINE(track, track_ar);

typedef struct {
        sqlite3_int64 id;

        str name;
        int64_t created_at;
        int64_t updated_at;
        track_ar tracks;
} playlist;

ARRAY_DEFINE(playlist, playlist_ar);

int db_init(const char *path);
void db_close(void);

void track_destroy(track *tr);
void playlist_destroy(playlist *pl);

int db_track_load(sqlite3_int64 id, track *tr);
int db_tracks_load(track_ar *tracks);

int db_playlist_load(sqlite3_int64 id, playlist *pl);
int db_playlists_load(playlist_ar *playlists);

#endif // DB_H_INCLUDED
