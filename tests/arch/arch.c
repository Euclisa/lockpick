#include <lockpick/arch/x86_64/bittestandset.h>
#include <lockpick/arch/x86_64/bittestandreset.h>
#include <lockpick/test.h>
#include <stdlib.h>


void test_intrin_bittestandset()
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


void test_intrin_bittestandreset()
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


void lp_test_intrin()
{
    LP_TEST_RUN(test_intrin_bittestandset());
    LP_TEST_RUN(test_intrin_bittestandreset());
}