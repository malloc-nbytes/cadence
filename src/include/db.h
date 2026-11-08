#ifndef DB_H_INCLUDED
#define DB_H_INCLUDED

#include "array.h"
#include "str.h"
#include "sqlite3.h"
#include "playlist.h"
#include "track.h"

#include <stdint.h>

int db_init(const char *path);
void db_close(void);

int db_track_load(sqlite3_int64 id, track *tr);
int db_tracks_load(track_ar *tracks);

int db_playlist_load(sqlite3_int64 id, playlist *pl);
int db_playlists_load(playlist_ar *playlists);

#endif // DB_H_INCLUDED
