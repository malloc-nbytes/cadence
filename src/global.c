#include "sqlite3.h"

#include <stddef.h>
#include <termios.h>

struct {
        struct {
                size_t w;
                size_t h;
                struct termios old;
        } term;

        sqlite3 *db;
} g_config = {
        .term = {
                .w = 0,
                .h = 0,
                .old = {0},
        },
        .db = NULL,
};
