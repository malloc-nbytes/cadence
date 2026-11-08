#ifndef COMMANDS_H_INCLUDED
#define COMMANDS_H_INCLUDED

#define COMMAND_MAIN_PLAYER    "player"
#define COMMAND_MAIN_SETTINGS  "settings"
#define COMMAND_MAIN_CREDITS   "credits"
#define COMMAND_MAIN_COPYING   "copying"
#define COMMAND_MAIN_QUIT      "quit"

#define COMMAND_MAIN_AS_CPL { \
        COMMAND_MAIN_PLAYER, \
        COMMAND_MAIN_SETTINGS, \
        COMMAND_MAIN_CREDITS, \
        COMMAND_MAIN_COPYING, \
        COMMAND_MAIN_QUIT, \
        NULL, \
}

#endif // COMMANDS_H_INCLUDED
