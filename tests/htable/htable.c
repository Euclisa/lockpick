#include <lockpick/htable.h>
#include <lockpick/container_of.h>
#include <lockpick/test.h>
#include <stdlib.h>


#define LP_TEST_HTABLE_UINT_ENTRY(uint_t,tests_num,entries_num)                                                  \
size_t uint_t##_hash(const void *x)                                                                     \
{                                                                                                       \
    uint_t *ux = (uint_t*)x;                                                                            \
    size_t a = 15429261971298606581ULL;                                                                 \
    size_t b = 14218188292255501262ULL;                                                                 \
    return (*ux)*a + b;                                                                                 \
}                                                                                                       \
bool uint_t##_eq(const void *a, const void *b)                                                          \
{                                                                                                       \
    uint_t *ua = (uint_t*)a;                                                                            \
    uint_t *ub = (uint_t*)b;                                                                            \
    return *ua == *ub;                                                                                  \
}                                                                                                       \
void shuffle_##uint_t(uint_t *arr, size_t length)                                                       \
{                                                                                                       \
    for(size_t i = 0; i < length * 2; ++i)                                                              \
    {                                                                                                   \
        size_t first_i = rand() % length;                                                               \
        size_t second_i = rand() % length;                                                              \
        lp_swap(*(arr+first_i),*(arr+second_i));                                                        \
    }                                                                                                   \
}                                                                                                       \
void test_htable_##uint_t()                                                                             \
{                                                                                                       \
    lp_htable_t *ht = lp_htable_create(sizeof(uint_t),1,&uint_t##_hash,&uint_t##_eq);                   \
    const size_t samples_num = MIN(1ULL << (sizeof(uint_t)*8-1),entries_num);                           \
    uint_t values[samples_num];                                                                         \
    for(size_t i = 0; i < samples_num; ++i)                                                             \
        values[i] = i;                                                                                  \
    for(size_t test_i = 0; test_i < tests_num; ++test_i)                                                \
    {                                                                                                   \
        shuffle_##uint_t(values,samples_num);                                                           \
        for(size_t i = 0; i < samples_num; ++i)                                                         \
        {                                                                                               \
            lp_htable_insert(ht,values+i);                                                              \
            bool valid = true;                                                                          \
            for(size_t j = 0; j < samples_num; ++j)                                                     \
            {                                                                                           \
                bool inserted_equal = false;                                                            \
                if(j <= i)                                                                              \
                    inserted_equal = (bool)lp_htable_insert(ht,values+j);                               \
                bool found = (bool)lp_htable_find(ht,values+j);                                         \
                size_t size = lp_htable_size(ht);                                                       \
                valid &= !inserted_equal &&                                                             \
                        found == (j <= i) &&                                                            \
                        size == i+1;                                                                    \
            }                                                                                           \
            LP_TEST_ASSERT(valid,"Htable inconsitency after %ld insertions",i);                         \
        }                                                                                               \
        shuffle_##uint_t(values,samples_num);                                                           \
        for(size_t i = 0; i < samples_num; ++i)                                                         \
        {                                                                                               \
            affirmf(lp_htable_remove(ht,values+i),"Failed to remove element from htable");              \
            bool valid = true;                                                                          \
            for(size_t j = 0; j < samples_num; ++j)                                                     \
            {                                                                                           \
                bool found = lp_htable_find(ht,values+j);                                               \
                size_t size = lp_htable_size(ht);                                                       \
                valid &= found == (j > i) &&                                                            \
                        size == samples_num-i-1;                                                        \
            }                                                                                           \
            LP_TEST_ASSERT(valid,"Htable inconsitency after %ld removals",i);                           \
        }                                                                                               \
    }                                                                                                   \
    lp_htable_release(ht);                                                                              \
}


LP_TEST_HTABLE_UINT_ENTRY(uint8_t,10,400)
LP_TEST_HTABLE_UINT_ENTRY(uint16_t,10,400)
LP_TEST_HTABLE_UINT_ENTRY(uint32_t,10,400)
LP_TEST_HTABLE_UINT_ENTRY(uint64_t,10,400)


void lp_test_htable()
{
    LP_TEST_RUN(test_htable_uint8_t());
    LP_TEST_RUN(test_htable_uint16_t());
    LP_TEST_RUN(test_htable_uint32_t());
    LP_TEST_RUN(test_htable_uint64_t());
}