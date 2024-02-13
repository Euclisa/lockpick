#include <lockpick/test.h>
#include <lockpick/math.h>
#include <math.h>


void test_math_is_pow_2()
{
    for(uint64_t exp = 0; exp < 14; ++exp)
    {
        uint64_t pow = 1 << exp;
        LP_TEST_ASSERT(lp_is_pow_2(pow),"%ld is power of two, but got false",pow);
        for(uint64_t value = pow+1; value < (pow << 1); ++value)
            LP_TEST_ASSERT(!lp_is_pow_2(value),"%ld is not power of 2, but got true");
    }
    lp_test_cleanup:
}


void test_math_floor_log2()
{
    for(uint32_t exp = 0; exp < 8; ++exp)
    {
        uint8_t pow = 1 << exp;
        for(uint8_t value = (pow >> 1)+1; value < pow; ++value)
        {
            uint32_t res_obt = lp_floor_log2(value);
            LP_TEST_ASSERT(res_obt == exp-1,"For %d expected: %d, got: %d",value,exp-1,res_obt);
        }
        uint32_t res_obt = lp_floor_log2(pow);
        LP_TEST_ASSERT(res_obt == exp,"For %d expected: %d, got: %d",pow,exp,res_obt);
    }

    for(uint32_t exp = 0; exp < 10; ++exp)
    {
        uint16_t pow = 1 << exp;
        for(uint16_t value = (pow >> 1)+1; value < pow; ++value)
        {
            uint32_t res_obt = lp_floor_log2(value);
            LP_TEST_ASSERT(res_obt == exp-1,"For %d expected: %d, got: %d",value,exp-1,res_obt);
        }
        uint32_t res_obt = lp_floor_log2(pow);
        LP_TEST_ASSERT(res_obt == exp,"For %d expected: %d, got: %d",pow,exp,res_obt);
    }

    for(uint32_t exp = 0; exp < 14; ++exp)
    {
        uint32_t pow = 1 << exp;
        for(uint32_t value = (pow >> 1)+1; value < pow; ++value)
        {
            uint32_t res_obt = lp_floor_log2(value);
            LP_TEST_ASSERT(res_obt == exp-1,"For %d expected: %d, got: %d",value,exp-1,res_obt);
        }
        uint32_t res_obt = lp_floor_log2(pow);
        LP_TEST_ASSERT(res_obt == exp,"For %d expected: %d, got: %d",pow,exp,res_obt);
    }

    for(uint64_t exp = 32; exp < 64; ++exp)
    {
        uint64_t value = 1ULL << exp;
        uint32_t res_obt = lp_floor_log2(value);
        LP_TEST_ASSERT(res_obt == exp,"For %d expected: %d, got: %d",value,exp,res_obt);
    }

    uint32_t exp = 32;
    uint64_t start = 1ULL << exp;
    uint64_t end = start + 1000;
    for(uint64_t value = start; value < end; ++value)
    {
        uint32_t res_obt = lp_floor_log2(value);
        LP_TEST_ASSERT(res_obt == exp,"For %ld expected: %d, got: %d",value,exp,res_obt);
    }
    lp_test_cleanup:
}


void test_math_ceil_log2()
{
    for(uint32_t exp = 0; exp < 8; ++exp)
    {
        uint8_t pow = 1 << exp;
        for(uint8_t value = (pow >> 1)+1; value < pow; ++value)
        {
            uint32_t res_obt = lp_ceil_log2(value);
            LP_TEST_ASSERT(res_obt == exp,"For %d expected: %d, got: %d",value,exp,res_obt);
        }
        uint32_t res_obt = lp_ceil_log2(pow);
        LP_TEST_ASSERT(res_obt == exp,"For %d expected: %d, got: %d",pow,exp,res_obt);
    }

    for(uint32_t exp = 0; exp < 10; ++exp)
    {
        uint16_t pow = 1 << exp;
        for(uint16_t value = (pow >> 1)+1; value < pow; ++value)
        {
            uint32_t res_obt = lp_ceil_log2(value);
            LP_TEST_ASSERT(res_obt == exp,"For %d expected: %d, got: %d",value,exp,res_obt);
        }
        uint32_t res_obt = lp_ceil_log2(pow);
        LP_TEST_ASSERT(res_obt == exp,"For %d expected: %d, got: %d",pow,exp,res_obt);
    }

    for(uint32_t exp = 0; exp < 14; ++exp)
    {
        uint32_t pow = 1 << exp;
        for(uint32_t value = (pow >> 1)+1; value < pow; ++value)
        {
            uint32_t res_obt = lp_ceil_log2(value);
            LP_TEST_ASSERT(res_obt == exp,"For %d expected: %d, got: %d",value,exp,res_obt);
        }
        uint32_t res_obt = lp_ceil_log2(pow);
        LP_TEST_ASSERT(res_obt == exp,"For %d expected: %d, got: %d",pow,exp,res_obt);
    }

    for(uint64_t exp = 32; exp < 64; ++exp)
    {
        uint64_t value = 1ULL << exp;
        uint32_t res_obt = lp_ceil_log2(value);
        LP_TEST_ASSERT(res_obt == exp,"For %d expected: %d, got: %d",value,exp,res_obt);
    }

    uint32_t exp = 32;
    uint64_t start = 1ULL << exp;
    uint64_t end = start + 1000;
    for(uint64_t value = start+1; value < end; ++value)
    {
        uint32_t res_obt = lp_ceil_log2(value);
        LP_TEST_ASSERT(res_obt == exp+1,"For %ld expected: %d, got: %d",value,exp+1,res_obt);
    }
    lp_test_cleanup:
}


void test_math_ceil_div_u()
{
    const uint64_t upper_bound = 1000;
    for(uint64_t a = 0; a <= upper_bound; ++a)
    {
        for(uint64_t b = 1; b <= upper_bound; ++b)
        {
            uint64_t true_ceil = (uint64_t)ceil((double)a/(double)b);
            uint64_t obt_ceil = lp_ceil_div_u(a,b);
            LP_TEST_ASSERT(obt_ceil == true_ceil,"Expected: %ld, got: %ld",true_ceil,obt_ceil);
        }
    }
    lp_test_cleanup:
}


void lp_test_math()
{
    LP_TEST_RUN(test_math_is_pow_2());
    LP_TEST_RUN(test_math_floor_log2());
    LP_TEST_RUN(test_math_ceil_log2());
    LP_TEST_RUN(test_math_ceil_div_u());
}