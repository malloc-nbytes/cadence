#ifndef TRIE_H_INCLUDED
#define TRIE_H_INCLUDED

#include <stddef.h>

#if defined(__GNUC__) || defined(__clang__)
#  define WARN_UNUSED_RESULT  __attribute__((warn_unused_result))
#else
#  define WARN_UNUSED_RESULT
#endif

// NOTE: The structure for the trie is an opaque type.

/**
 * Returns: a new prefix trie or NULL on failure
 * Description: Allocate a new prefix trie. This datastructure
 *              can be used for autocompletions of strings.
 */
void *trie_alloc(void) WARN_UNUSED_RESULT;

/**
 * Parameter: t    -> the trie
 * Parameter: word -> a word to autocomplete
 * Returns: 1 on success, 0 on failure
 * Description: Insert a new word for autocompletion
 *              into the trie.
 */
int trie_insert(void *t, const char *word);

/**
 * Parameter: t           -> the trie
 * Parameter: prefix      -> the prefix to lookup
 * Parameter: max_results -> max number of results returned
 * Parameter: out_count   -> the number of items found.
 * Returns: A string array of all autocompleted words. The length
 *          of this array is stored in `out_count'.
 * Description: Get an array of all words that can be completed
 *              from the prefix `prefix'.
 */
char **trie_get_completions(void       *t,
                            const char *prefix,
                            size_t      max_results,
                            size_t     *out_count);

/**
 * Parameter: t -> the trie
 * Description: Free all memory alloc'd by the trie.
 */
void trie_destroy(void *t);

#endif // TRIE_H_INCLUDED
