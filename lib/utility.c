#include <lockpick/utility.h>
#include <lockpick/math.h>


inline size_t lp_uni_hash(size_t x)
{
    const size_t a = 1011731769645795246ULL;
    const size_t b = 6675104919954798621ULL;
    const uint64_t exp = 28;

    size_t h = lp_pow_u64(x,exp)*a + b;

    return h;
}