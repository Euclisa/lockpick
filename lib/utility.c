#include <lockpick/utility.h>


inline size_t lp_uni_hash(size_t x)
{
    const size_t a = 1011731769645795246ULL;
    const size_t b = 6675104919954798621ULL;

    size_t h = x*a + b;

    return h;
}