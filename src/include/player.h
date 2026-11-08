#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include "playlist.h"

#include <stdint.h>
#include <stddef.h>

typedef struct context {
        playlist_ar playlists;
        uint8_t volume;
        size_t (*next_track_fun)(const struct context *);
        size_t next_track;
} context;

void player_run(void);

#endif // PLAYER_H_INCLUDED
