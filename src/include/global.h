#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include "sqlite3.h"

#include <stddef.h>
#include <termios.h>

extern struct {
        struct {
                size_t w;
                size_t h;
                struct termios old;
        } term;
        sqlite3 *db;
} g_config;

#endif // GLOBAL_H_INCLUDED
