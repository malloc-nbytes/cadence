#include "global.h"
#include "db.h"
#include "term.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void
cleanup(void)
{
        disable_raw_terminal(STDIN_FILENO, &g_config.term.old);
        term_exit_fullscrn();
        show_cursor();
}

static int
init(void)
{
        if (!db_init("cadence.db"))
                return 0;

        if (!get_terminal_xy(&g_config.term.w, &g_config.term.h))
                return 0;

        if (!enable_raw_terminal(STDIN_FILENO, &g_config.term.old))
                return 0;

        clear_terminal();
        term_fullscrn();
        hide_cursor();

        return 1;
}

int
main(void)
{
        if (!init()) {
                fprintf(stderr, "could not initialize terminal\n");
                return 1;
        }
        atexit(cleanup);

        return 0;
}

