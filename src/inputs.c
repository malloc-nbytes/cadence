#include "inputs.h"
#include "global.h"
#include "term.h"
#include "str.h"
#include "trie.h"
#include "colors.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void
draw_line_to_term_width(void)
{
        for (size_t i = 0; i < g_config.term.w; ++i)
                putchar('-');
}

char *
get_input_in_minibuffer(const char *prompt, cstr_ar ac)
{
        str res;
        size_t x;
        size_t prompt_n;
        void *trie;
        int autofill;
        size_t ac_selected;

        res = str_create();
        x = 0;
        if (!prompt)
                prompt = "/";
        prompt_n = strlen(prompt);
        trie = trie_alloc();
        autofill = 0;
        ac_selected = 0;

        for (size_t i = 0; i < ac.len; ++i)
                trie_insert(trie, ac.data[i]);

        show_cursor();

        gotoxy(0, g_config.term.h-2);
        draw_line_to_term_width();

        while (1) {
        redraw:
                gotoxy(0, g_config.term.h-1);
                clear_line_imm();

                if (autofill)
                        printf(BOLD "%s" RESET, prompt);
                else
                        printf("%s", prompt);

                if (res.len > 0)
                        printf("%s", res.chars);

                size_t out_count;
                char **found = trie_get_completions(trie, res.chars, 7, &out_count);

                if (ac_selected > out_count)
                        ac_selected = out_count-1;

                if (out_count > 1)
                        putchar('{');

                for (size_t i = 0; i < out_count; ++i) {
                        if (i)
                                putchar(' ');
                        if (i == ac_selected)
                                printf(UNDERLINE YELLOW "%s" RESET, found[i]+res.len);
                        else
                                printf(GRAY "%s" RESET, found[i]+res.len);
                }

                if (out_count > 1)
                        putchar('}');

                if (autofill && found && found[0]) {
                        str_overwrite(&res, found[ac_selected]);
                        x = res.len;
                        autofill = 0;
                        ac_selected = 0;
                        goto redraw;
                }

                gotoxy(0, g_config.term.h);
                draw_line_to_term_width();
                gotoxy(x+prompt_n, g_config.term.h-1);

                fflush(stdout);

                char ch;
                input_type ty;

                switch (ty = get_input(&ch)) {
                case INPUT_TYPE_CTRL: {
                        if (ch == CTRL_F && x < res.len)
                                ++x;
                        else if (TAB(ch)) {
                                autofill = !autofill;
                        }
                        else if (ch == CTRL_B && x > 0)
                                --x;
                        else if (ch == CTRL_A)
                                x = 0;
                        else if (ch == CTRL_E)
                                x = res.len;
                        else if (ch == CTRL_K)
                                str_cut(&res, x);
                        else if (ch == CTRL_D && x != res.len)
                                str_rm(&res, x);
                        else if (ch == CTRL_N && ac.len > 0)
                                ++ac_selected;
                        else if (ch == CTRL_P && ac.len > 0 && ac_selected > 0)
                                --ac_selected;
                        else if (ch == CTRL_G)
                                goto done;
                } break;
                case INPUT_TYPE_NORMAL: {
                        if (BACKSPACE(ch) && x > 0) {
                                --x;
                                str_rm(&res, x);
                        }
                        else if (ENTER(ch))
                                goto done;
                        else if (isprint(ch))
                                str_insert(&res, x++, ch);
                } break;
                default:
                        break;
                }
        }

done:
        hide_cursor();

        for (int i = 2; i >= 0; --i) {
                gotoxy(0, g_config.term.h-i);
                clear_line_imm();
        }

        if (res.len == 0) {
                str_destroy(&res);
                return NULL;
        }

        return res.chars;
}
