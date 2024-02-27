#include <lockpick/math.h>


inline uint64_t lp_ceil_div_u64(uint64_t a, uint64_t b)
{
    return (a / b) + !!(a % b);
}


uint64_t lp_pow_u64(uint64_t base, uint64_t exp)
{
    if(exp == 0)
        return 1;

    uint64_t res = 1;
    for(int16_t shift = lp_floor_log2(exp); shift >= 0; --shift)
    {
        res *= res;
        uint64_t curr_exp_bit = (exp >> shift) & 1ULL;
        if(curr_exp_bit)
            res *= base;
    }

    return res;
}