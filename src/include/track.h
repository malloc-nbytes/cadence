#ifndef TRACK_H_INCLUDED
#define TRACK_H_INCLUDED

#include "array.h"
#include "str.h"
#include "sqlite3.h"

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

void track_destroy(track *tr);

#endif // TRACK_H_INCLUDED
