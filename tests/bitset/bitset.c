#include <lockpick/test.h>
#include <lockpick/bitset.h>
#include <stdlib.h>


void test_bitset_random_set_reset()
{
    const uint32_t bitset_size = 10000;

    lp_bitset_t *bits = lp_bitset_create(bitset_size);
    bool samp_bits[bitset_size];
    uint32_t true_count = 0;

    LP_TEST_ASSERT(
        !lp_bitset_any(bits) &&
        !lp_bitset_all(bits) &&
        lp_bitset_none(bits),
        "Bits number conditional failed after creation");
    
    for(uint32_t bit_i = 0; bit_i < bitset_size; ++bit_i)
    {
        bool bit_value = (bool)(rand()%2);
        samp_bits[bit_i] = bit_value;

        bool obt_bit;
        if(bit_value)
            obt_bit = lp_bitset_set(bits,bit_i);
        else
            obt_bit = lp_bitset_reset(bits,bit_i);

        true_count += bit_value;
        uint32_t obt_count = lp_bitset_count(bits);

        LP_TEST_ASSERT(
            lp_bitset_any(bits) == !!true_count &&
            lp_bitset_all(bits) == (true_count == bitset_size) &&
            lp_bitset_none(bits) == !true_count,
            "Bits number conditional failed at bit index %d at init random set",bit_i);
        LP_TEST_ASSERT(obt_bit == false,
            "Expected zero initialized bitset, but 1 found at bit index %d",bit_i);
        LP_TEST_ASSERT(true_count == obt_count,
            "Expected count: %d, got: %d, at init random set",true_count,obt_count);
    }

    for(uint32_t bit_i = 0; bit_i < bitset_size; ++bit_i)
    {
        bool true_bit = samp_bits[bit_i];
        bool obt_bit = lp_bitset_reset(bits,bit_i);

        if(true_bit)
            --true_count;
        uint32_t obt_count = lp_bitset_count(bits);

        LP_TEST_ASSERT(
            lp_bitset_any(bits) == !!true_count &&
            lp_bitset_all(bits) == (true_count == bitset_size) &&
            lp_bitset_none(bits) == !true_count,
            "Bits number conditional failed at bit index %d at final reset",bit_i);
        LP_TEST_ASSERT(true_bit == obt_bit,
            "Expected: %d, got: %d at bit index %d",true_bit,obt_bit,bit_i);
        LP_TEST_ASSERT(true_count == obt_count,
            "Expected count: %d, got: %d, at final reset",true_count,obt_count);
    }

    lp_test_cleanup:
    lp_bitset_release(bits);
}


void test_bitset_random_update()
{
    const uint32_t bitset_size = 10000;

    lp_bitset_t *bits = lp_bitset_create(bitset_size);
    bool samp_bits[bitset_size];
    uint32_t true_count = 0;

    LP_TEST_ASSERT(
        !lp_bitset_any(bits) &&
        !lp_bitset_all(bits) &&
        lp_bitset_none(bits),
        "Bits number conditional failed after creation");
    
    for(uint32_t bit_i = 0; bit_i < bitset_size; ++bit_i)
    {
        bool bit_value = (bool)(rand()%2);
        samp_bits[bit_i] = bit_value;

        bool obt_bit = lp_bitset_update(bits,bit_i,bit_value);

        true_count += bit_value;
        uint32_t obt_count = lp_bitset_count(bits);

        LP_TEST_ASSERT(
            lp_bitset_any(bits) == !!true_count &&
            lp_bitset_all(bits) == (true_count == bitset_size) &&
            lp_bitset_none(bits) == !true_count,
            "Bits number conditional failed at bit index %d at init random set",bit_i);
        LP_TEST_ASSERT(obt_bit == false,
            "Expected zero initialized bitset, but 1 found at bit index %d",bit_i);
        LP_TEST_ASSERT(true_count == obt_count,
            "Expected count: %d, got: %d, at init random set",true_count,obt_count);
    }

    for(uint32_t bit_i = 0; bit_i < bitset_size; ++bit_i)
    {
        bool true_bit = samp_bits[bit_i];
        bool obt_bit = lp_bitset_reset(bits,bit_i);

        if(true_bit)
            --true_count;
        uint32_t obt_count = lp_bitset_count(bits);

        LP_TEST_ASSERT(
            lp_bitset_any(bits) == !!true_count &&
            lp_bitset_all(bits) == (true_count == bitset_size) &&
            lp_bitset_none(bits) == !true_count,
            "Bits number conditional failed at bit index %d at final reset",bit_i);
        LP_TEST_ASSERT(true_bit == obt_bit,
            "Expected: %d, got: %d at bit index %d",true_bit,obt_bit,bit_i);
        LP_TEST_ASSERT(true_count == obt_count,
            "Expected count: %d, got: %d, at final reset",true_count,obt_count);
    }

    lp_test_cleanup:
    lp_bitset_release(bits);
}


void test_bitset_set_reset_all()
{
    const uint32_t bitset_size = 10000;

    lp_bitset_t *bits = lp_bitset_create(bitset_size);

    LP_TEST_ASSERT(
        !lp_bitset_any(bits) &&
        !lp_bitset_all(bits) &&
        lp_bitset_none(bits),
        "Bits number conditional failed after creation");

    lp_bitset_set_all(bits);

    LP_TEST_ASSERT(
        lp_bitset_any(bits) &&
        lp_bitset_all(bits) &&
        !lp_bitset_none(bits),
        "Bits number conditional failed after all bits set");
    
    uint32_t obt_count = lp_bitset_count(bits);
    LP_TEST_ASSERT(obt_count == bitset_size,
        "Expected count: %d, got: %d after all bits set",bitset_size,obt_count);

    for(uint32_t bit_i = 0; bit_i < bitset_size; ++bit_i)
        LP_TEST_ASSERT(lp_bitset_test(bits,bit_i) == true,
            "Expected all bits set, but found 0 at index %d",bit_i);
    
    lp_bitset_reset_all(bits);

    LP_TEST_ASSERT(
        !lp_bitset_any(bits) &&
        !lp_bitset_all(bits) &&
        lp_bitset_none(bits),
        "Bits number conditional failed after all bits reset");

    obt_count = lp_bitset_count(bits);
    LP_TEST_ASSERT(obt_count == 0,
        "Expected count: 0, got: %d after all bits reset",obt_count);
    
    for(uint32_t bit_i = 0; bit_i < bitset_size; ++bit_i)
        LP_TEST_ASSERT(lp_bitset_test(bits,bit_i) == false,
            "Expected all bits reset, but found 1 at index %d",bit_i);

    lp_test_cleanup:
    lp_bitset_release(bits);
}


void test_bitset_copy()
{
    const uint32_t a_size = 1000;
    lp_bitset_t *a = lp_bitset_create(a_size);

    const uint32_t b_size = 100;
    lp_bitset_t *b = lp_bitset_create(b_size);

    for(uint32_t bit_i = 0; bit_i < b_size; ++bit_i)
        lp_bitset_update(b,bit_i,(bool)(rand()%2));
    
    lp_bitset_copy(a,b);

    for(uint32_t bit_i = 0; bit_i < b_size; ++bit_i)
    {
        bool a_bit_value = lp_bitset_test(a,bit_i);
        bool b_bit_value = lp_bitset_test(b,bit_i);

        LP_TEST_ASSERT(a_bit_value == b_bit_value,
            "Expected %d in destination bitset A, got %d at bit %d",
            (uint32_t)b_bit_value,(uint32_t)a_bit_value,bit_i);
    }

    for(uint32_t bit_i = 0; bit_i < a_size; ++bit_i)
        lp_bitset_update(a,bit_i,(bool)(rand()%2));
    
    lp_bitset_copy(b,a);
    
    for(uint32_t bit_i = 0; bit_i < b_size; ++bit_i)
    {
        bool a_bit_value = lp_bitset_test(a,bit_i);
        bool b_bit_value = lp_bitset_test(b,bit_i);

        LP_TEST_ASSERT(a_bit_value == b_bit_value,
            "Expected %d in destination bitset B, got %d at bit %d",
            (uint32_t)a_bit_value,(uint32_t)b_bit_value,bit_i);
    }

    lp_test_cleanup:
    lp_bitset_release(a);
    lp_bitset_release(b);
}


void lp_test_bitset()
{
    LP_TEST_RUN(test_bitset_random_set_reset());
    LP_TEST_RUN(test_bitset_random_update());
    LP_TEST_RUN(test_bitset_set_reset_all());
    LP_TEST_RUN(test_bitset_copy());
}
