#include <strings.h>

#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "errnos.h"
#include <err.h>

#define VLA_MAXLEN (50 * sizeof(void*))

/**
 * START_VLA automatically switched between VLA and malloc.
 *
 * It must be put in a single statement.
 *
 * There can only be one START_VLA and one END_VLA in one scope.
 */
#define START_VLA(type, n, varname)                  \
    type vla[n * sizeof(type) > VLA_MAXLEN ? 0 : n]; \
    if (sizeof(vla) == 0) {                          \
        varname = malloc(n * sizeof(type));          \
        if (varname == NULL) {                       \
            warnx("malloc %zu failed", n * sizeof(type)); \
            return (EXECUTION_FAILURE);              \
        }                                            \
    } else                                           \
        varname = vla

/**
 * START_VLA2 is almost the same as START_VLA except that it 
 * initializes the array to 0.
 */
#define START_VLA2(type, n, varname)                 \
    type vla[n * sizeof(type) > VLA_MAXLEN ? 0 : n]; \
    do {                                             \
        if (sizeof(vla) != 0) {                      \
            varname = vla;                           \
            memset(vla, 0, sizeof(vla));             \
        } else {                                     \
            varname = calloc(n, sizeof(type));       \
            if (varname == NULL) {                   \
                warnx("calloc %zu failed", n * sizeof(type)); \
                return (EXECUTION_FAILURE);          \
            }                                        \
        }                                            \
    } while (0)

/**
 * END_VLA must be put in a single statement.
 */
#define END_VLA(varname)  \
    if (sizeof(vla) == 0) \
        (free)(varname)

#define STR_IMPL_(x) #x      //stringify argument
#define STR(x) STR_IMPL_(x)  //indirection to expand argument macros

uintmax_t min_unsigned(uintmax_t x, uintmax_t y)
{
    return x > y ? y : x;
}

/**
 * @param str must not be null
 * @param integer must be a valid pointer.
 *                If str2int failed, its value is unchanged.
 * @return 0 on success, -1 if not integer, -2 if out of range.
 *
 * NOTE that this function does not call builtin_usage on error.
 */
int str2int(const char *str, int *integer)
{
    intmax_t result;
    if (legal_number(str, &result) == 0) {
        return -1;
    } else if (result > INT_MAX || result < INT_MIN)
        return -2;
 
    *integer = result;
    return 0;
}

/**
 * @param str must not be null
 * @param integer must be a valid pointer.
 *                If str2uint failed, its value is unchanged.
 * @return 0 on success, -1 if not integer, -2 if too large.
 *
 * NOTE that this function does not call builtin_usage on error.
 */
int str2uint(const char *str, unsigned *integer)
{
    intmax_t result;
    if (legal_number(str, &result) == 0)
        return -1;
    else if (result > UINT_MAX || result < 0)
        return -2;
 
    *integer = result;
    return 0;
}

/**
 * @param str must not be null
 * @param integer must be a valid pointer.
 *                If str2uint32 failed, its value is unchanged.
 * @return 0 on success, -1 if not integer, -2 if too large.
 *
 * NOTE that this function does not call builtin_usage on error.
 */
int str2uint32(const char *str, uint32_t *integer)
{
    intmax_t result;
    if (legal_number(str, &result) == 0)
        return -1;
    else if (result > UINT32_MAX || result < 0)
        return -2;
 
    *integer = result;
    return 0;
}

/**
 * @param str must not be null
 * @param integer must be a valid pointer.
 *                If str2pint failed, its value is unchanged.
 * @return 0 on success, -1 if not integer, -2 if out or range.
 *
 * convert str to positive int.
 *
 * NOTE that this function does not call builtin_usage on error.
 */
int str2pint(const char *str, int *integer)
{
    int result = str2int(str, integer);
    if (result < 0)
        return result;
    if (*integer < 0)
        return -2;
    return 0;
}

/**
 * @param str must not be null
 * @param fd must be a valid pointer.
 *           If str2fd failed, its value is unchanged.
 * @return 0 on success, -1 if not enough/too many arguments.
 */
int str2fd(const char *str, int *fd)
{
    int result = str2pint(str, fd);
    if (result == -1) {
        builtin_usage();
        return -1;
    } else if (result == -2) {
        warnx("Input fd too large!");
        return -1;
    }
    return 0;
}




/**
 * @return number of args read in.
 */
int readin_args(WORD_LIST **l, int argc, const char *argv[])
{
    int i = 0;
    for (; i != argc && (*l) != NULL; ++i) {
        argv[i] = (*l)->word->word;
        (*l) = (*l)->next;
    }
    return i;
}

/**
 * @return number of optional arg read in on success, -1 if not enough/too many arguments.
 */
int to_argv_opt(WORD_LIST *l, int argc, int opt_argc, const char *argv[])
{
    if (readin_args(&l, argc, argv) < argc) {
        builtin_usage();
        return -1;
    }

    int i = readin_args(&l, opt_argc, argv + argc);
    if (l != NULL) {
        builtin_usage();
        return -1;
    }
    return i;
}
/**
 * @return 0 on success, -1 if not enough/too many arguments.
 */
int to_argv(WORD_LIST *l, int argc, const char *argv[])
{
    return to_argv_opt(l, argc, 0, argv);
}

/**
 * @return -1 if failed, 0 if succeeds.
 *
 * This function call will also reset_internal_getopt and set list = loptend
 * for you.
 */
int check_no_options(WORD_LIST **list)
{
    reset_internal_getopt();
    if (no_options(*list)) // If options present
        return -1;
    *list = loptend;
    return 0;
}
