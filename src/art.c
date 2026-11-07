#include "art.h"
#include "global.h"
#include "term.h"

#include <stdio.h>
#include <string.h>

void
print_cadence_art(void)
{
        static const char *const art[] = {
                "  CCCCC    AAAAA   DDDDDD   EEEEEEE  NN    NN   CCCCC   EEEEEEE",
                " CC       AA   AA  DD   DD  EE       NNN   NN  CC       EE     ",
                "CC       AA     AA DD    DD EEEEE    NNNN  NN CC        EEEEE  ",
                "CC       AAAAAAAAA DD    DD EEEEE    NN NN NN CC        EEEEE  ",
                " CC      AA     AA DD   DD  EE       NN  NNNN  CC       EE     ",
                "  CCCCC  AA     AA DDDDDD   EEEEEEE  NN   NNN   CCCCC   EEEEEEE",
        };
        size_t art_width = 0;
        size_t art_height = sizeof(art) / sizeof(art[0]);
        size_t x;
        size_t y;

        for (size_t i = 0; i < art_height; ++i) {
                size_t width = strlen(art[i]);

                if (width > art_width)
                        art_width = width;
        }

        x = g_config.term.w > art_width ?
            (g_config.term.w - art_width) / 2 : 0;
        y = g_config.term.h > art_height ?
            (g_config.term.h - art_height) / 2 : 0;

        for (size_t i = 0; i < art_height; ++i) {
                gotoxy(x, y + i);
                fputs(art[i], stdout);
        }
        fflush(stdout);
}
