#include <lockpick/utility.h>
#include <lockpick/math.h>
#include <lockpick/affirmf.h>


inline char lp_digit_to_char(uint16_t digit)
{
    affirmf(digit <= 9,"Digit must be in range 0-9, but %d was given",digit);

    return '0'+digit;
}


inline size_t lp_uni_hash(size_t x)
{
    const size_t a = 1011731769645795246ULL;
    const size_t b = 6675104919954798621ULL;
    const uint64_t exp = 28;

    size_t h = lp_pow_u64(x,exp)*a + b;

    return h;
}