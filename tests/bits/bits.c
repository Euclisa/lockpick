#include <lockpick/bits.h>
#include <lockpick/sync/bits.h>
#include <lockpick/test.h>
#include <stdlib.h>


void test_bits_bittestandset()
{
    const uint32_t tests_num = 10000;
    const uint32_t samp_words_num = 16;
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        uint32_t samp[samp_words_num];
        uint32_t samp_c[samp_words_num];
        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
            samp[word_i] = samp_c[word_i] = rand();

        uint32_t test_set_samp[samp_words_num];
        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
        {
            test_set_samp[word_i] = 0;
            for(uint8_t shift = 0; shift < 32; ++shift)
                test_set_samp[word_i] |= !!lp_bittestandset(&samp[0],word_i*32+shift) << shift;
        }
        
        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
            LP_TEST_ASSERT(samp[word_i] == (uint32_t)-1ULL && test_set_samp[word_i] == samp_c[word_i],
                    "Got sample after all bits set: %d; Test sample bitmask: %d in word %d; Expected word: %d",
                    samp[word_i],test_set_samp[word_i],word_i,samp_c[word_i]);
    }
    lp_test_cleanup:
}


void test_bits_bittestandreset()
{
    const uint32_t tests_num = 10000;
    const uint32_t samp_words_num = 16;
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        uint32_t samp[samp_words_num];
        uint32_t samp_c[samp_words_num];
        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
            samp[word_i] = samp_c[word_i] = rand();

        uint32_t test_set_samp[samp_words_num];
        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
        {
            test_set_samp[word_i] = 0;
            for(uint8_t shift = 0; shift < 32; ++shift)
                test_set_samp[word_i] |= !!lp_bittestandreset(&samp[0],word_i*32+shift) << shift;
        }
        
        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
            LP_TEST_ASSERT(samp[word_i] == 0 && test_set_samp[word_i] == samp_c[word_i],
                    "Got sample after all bits reset: %d; Test sample bitmask: %d in word %d; Expected word: %d",
                    samp[word_i],test_set_samp[word_i],word_i,samp_c[word_i]);
    }
    lp_test_cleanup:
}


void test_bits_atomic_bittestandset()
{
    const uint32_t tests_num = 10000;
    const uint32_t samp_words_num = 16;
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        uint32_t samp[samp_words_num];
        uint32_t samp_c[samp_words_num];
        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
            samp[word_i] = samp_c[word_i] = rand();

        uint32_t test_set_samp[samp_words_num];
        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
        {
            test_set_samp[word_i] = 0;
            for(uint8_t shift = 0; shift < 32; ++shift)
                test_set_samp[word_i] |= !!lp_atomic_bittestandset(&samp[0],word_i*32+shift) << shift;
        }
        
        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
            LP_TEST_ASSERT(samp[word_i] == (uint32_t)-1ULL && test_set_samp[word_i] == samp_c[word_i],
                    "Got sample after all bits set: %d; Test sample bitmask: %d in word %d; Expected word: %d",
                    samp[word_i],test_set_samp[word_i],word_i,samp_c[word_i]);
    }
    lp_test_cleanup:
}


void test_bits_atomic_bittestandreset()
{
    const uint32_t tests_num = 10000;
    const uint32_t samp_words_num = 16;
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        uint32_t samp[samp_words_num];
        uint32_t samp_c[samp_words_num];
        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
            samp[word_i] = samp_c[word_i] = rand();

        uint32_t test_set_samp[samp_words_num];
        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
        {
            test_set_samp[word_i] = 0;
            for(uint8_t shift = 0; shift < 32; ++shift)
                test_set_samp[word_i] |= !!lp_atomic_bittestandreset(&samp[0],word_i*32+shift) << shift;
        }
        
        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
            LP_TEST_ASSERT(samp[word_i] == 0 && test_set_samp[word_i] == samp_c[word_i],
                    "Got sample after all bits reset: %d; Test sample bitmask: %d in word %d; Expected word: %d",
                    samp[word_i],test_set_samp[word_i],word_i,samp_c[word_i]);
    }
    lp_test_cleanup:
}


void test_bits_bittest()
{
    const uint32_t tests_num = 10000;
    const uint32_t samp_words_num = 16;
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        uint32_t samp[samp_words_num];
        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
            samp[word_i] = rand();
        
        uint32_t test_samp[samp_words_num];
        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
        {
            test_samp[word_i] = 0;
            for(uint8_t shift = 0; shift < 32; ++shift)
                test_samp[word_i] |= !!lp_bittest(&samp[0],word_i*32+shift) << shift;
        }

        for(uint32_t word_i = 0; word_i < samp_words_num; ++word_i)
            LP_TEST_ASSERT(test_samp[word_i] == samp[word_i],
                "Expected word at index %d to be: %d, got: %d",
                word_i,samp[word_i],test_samp[word_i]);
    }
    lp_test_cleanup:
}


void lp_test_bits()
{
    LP_TEST_RUN(test_bits_bittestandset());
    LP_TEST_RUN(test_bits_atomic_bittestandset());

    LP_TEST_RUN(test_bits_bittestandreset());
    LP_TEST_RUN(test_bits_atomic_bittestandreset());

    LP_TEST_RUN(test_bits_bittest());
}