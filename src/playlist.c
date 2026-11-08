#include "playlist.h"

void
playlist_destroy(playlist *pl)
{
        for (size_t i = 0; i < pl->tracks.len; ++i)
                track_destroy(&pl->tracks.data[i]);
        array_free(pl->tracks);
        str_destroy(&pl->name);
}

