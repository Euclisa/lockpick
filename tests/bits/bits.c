#include <lockpick/bits.h>
#include <lockpick/sync/bits.h>
#include <lockpick/test.h>
#include <stdlib.h>


void test_bitops_bittestandset()
{
    const uint32_t tests_num = 100000;
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        uint32_t samp = rand();
        uint32_t samp_c = samp;
        uint32_t test_set_samp = 0;
        for(uint8_t shift = 0; shift < 32; ++shift)
            test_set_samp |= !!lp_bittestandset(&samp,shift) << shift;
        LP_TEST_ASSERT(samp == (uint32_t)-1 && test_set_samp == samp_c,
                "Got sample after all bits set: %d; Test sample bitmask: %d",samp,test_set_samp);
    }
    lp_test_cleanup:
}


void test_bitops_bittestandreset()
{
    const uint32_t tests_num = 100000;
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        uint32_t samp = rand();
        uint32_t samp_c = samp;
        uint32_t test_set_samp = 0;
        for(uint8_t shift = 0; shift < 32; ++shift)
            test_set_samp |= !!lp_bittestandreset(&samp,shift) << shift;
        LP_TEST_ASSERT(samp == 0 && test_set_samp == samp_c,
                "Got sample after all bits set: %d; Test sample bitmask: %d",samp,test_set_samp);
    }
    lp_test_cleanup:
}


void test_bitops_atomic_bittestandset()
{
    const uint32_t tests_num = 100000;
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        uint32_t samp = rand();
        uint32_t samp_c = samp;
        uint32_t test_set_samp = 0;
        for(uint8_t shift = 0; shift < 32; ++shift)
            test_set_samp |= !!lp_atomic_bittestandset(&samp,shift) << shift;
        LP_TEST_ASSERT(samp == (uint32_t)-1 && test_set_samp == samp_c,
                "Got sample after all bits set: %d; Test sample bitmask: %d",samp,test_set_samp);
    }
    lp_test_cleanup:
}


void test_bitops_atomic_bittestandreset()
{
    const uint32_t tests_num = 100000;
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        uint32_t samp = rand();
        uint32_t samp_c = samp;
        uint32_t test_set_samp = 0;
        for(uint8_t shift = 0; shift < 32; ++shift)
            test_set_samp |= !!lp_atomic_bittestandreset(&samp,shift) << shift;
        LP_TEST_ASSERT(samp == 0 && test_set_samp == samp_c,
                "Got sample after all bits set: %d; Test sample bitmask: %d",samp,test_set_samp);
    }
    lp_test_cleanup:
}



void lp_test_bitops()
{
    LP_TEST_RUN(test_bitops_bittestandset());
    LP_TEST_RUN(test_bitops_atomic_bittestandset());

    LP_TEST_RUN(test_bitops_bittestandreset());
    LP_TEST_RUN(test_bitops_atomic_bittestandreset());
}