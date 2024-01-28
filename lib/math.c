#include <lockpick/math.h>


inline uint64_t lp_ceil_div_u(uint64_t a, uint64_t b)
{
    return (a / b) + !!(a % b);
}