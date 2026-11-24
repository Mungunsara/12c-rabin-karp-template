#include "search.h"
#include <string.h>

#define BASE 256ULL

/* TODO 1: Polynomial hash */
uint64_t rk_hash(const unsigned char *data, size_t length)
{
    uint64_t hash = 0;

    if (length == 0)
        return 0;

    for (size_t i = 0; i < length; i++) {
        hash = (hash * BASE + data[i]) % RK_MOD;
    }

    return hash;
}

/* TODO 2: Rolling hash */
uint64_t rk_roll(uint64_t current, unsigned char outgoing,
                 unsigned char incoming, uint64_t high_power)
{
    uint64_t removed =
        ((uint64_t)outgoing * high_power) % RK_MOD;

    current = (current + RK_MOD - removed) % RK_MOD;

    current = (current * BASE + incoming) % RK_MOD;

    return current;
}

/* TODO 3: Search all matches */
int search_all(const unsigned char *text, size_t n,
               const unsigned char *pattern, size_t m,
               match_fn emit, void *context)
{
    /* Argument validation */
    if (m == 0 || pattern == NULL || emit == NULL ||
        (n > 0 && text == NULL)) {
        return SEARCH_BAD_ARGUMENT;
    }

    /* Empty text or pattern longer than text */
    if (n == 0 || m > n) {
        return SEARCH_OK;
    }

    /* Calculate highest power: BASE^(m-1) % MOD */
    uint64_t high_power = 1;

    for (size_t i = 1; i < m; i++) {
        high_power = (high_power * BASE) % RK_MOD;
    }

    /* Calculate pattern and first window hashes */
    uint64_t pattern_hash = rk_hash(pattern, m);
    uint64_t window_hash = rk_hash(text, m);

    /* Check every possible window */
    for (size_t i = 0; ; i++) {

        /* Hash match: verify actual bytes */
        if (window_hash == pattern_hash &&
            memcmp(text + i, pattern, m) == 0) {

            if (emit(i, context) != 0) {
                return SEARCH_OUTPUT_ERROR;
            }
        }

        /* Last possible window */
        if (i == n - m) {
            break;
        }

        /* Move window by one byte */
        window_hash = rk_roll(
            window_hash,
            text[i],
            text[i + m],
            high_power
        );
    }

    return SEARCH_OK;
}
