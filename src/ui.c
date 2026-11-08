#include "ui.h"
#include "term.h"

#include <stdio.h>

void
draw_hdivider(size_t x, size_t y, size_t untilx)
{
        gotoxy(x, y);

        for (size_t i = x; i < untilx; ++i)
                putchar('-');
}

void
draw_vdivider(size_t x, size_t y, size_t untily)
{
        gotoxy(x, y);

        for (size_t i = y; i < untily; ++i)
                putchar('-');
}
