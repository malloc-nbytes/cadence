#include "global.h"
#include "array.h"
#include "db.h"
#include "player.h"
#include "term.h"
#include "commands.h"
#include "inputs.h"
#include "art.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

        print_cadence_art();
        printf("\nType `/' to begin");
        fflush(stdout);

        cstr_ar autocomplete_commands = array_empty();
        static char *const cmds[] = COMMAND_MAIN_AS_CPL;
        for (size_t i = 0; cmds[i]; ++i)
                array_append(autocomplete_commands, cmds[i]);

        while (1) {
                char *option;
                if (!(option = get_input_in_minibuffer("/", autocomplete_commands)))
                        continue;
                if (!strcmp(option, COMMAND_MAIN_QUIT))
                        break;
                if (!strcmp(option, COMMAND_MAIN_PLAYER))
                        player_run();
        }

        array_free(autocomplete_commands);

        return 0;
}

