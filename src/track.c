#include "track.h"

void
track_destroy(track *tr)
{
        str_destroy(&tr->artist);
        str_destroy(&tr->album);
        str_destroy(&tr->name);
        str_destroy(&tr->path);
}
