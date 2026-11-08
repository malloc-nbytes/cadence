#ifndef PLAYLIST_H_INCLUDED
#define PLAYLIST_H_INCLUDED

#include "array.h"
#include "str.h"
#include "track.h"
#include "sqlite3.h"

typedef struct {
        sqlite3_int64 id;

        str name;
        int64_t created_at;
        int64_t updated_at;
        track_ar tracks;
} playlist;

ARRAY_DEFINE(playlist, playlist_ar);

void playlist_destroy(playlist *pl);

#endif // PLAYLIST_H_INCLUDED
