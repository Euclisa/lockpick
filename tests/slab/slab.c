#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <lockpick/test.h>
#include <lockpick/slab.h>
#include <lockpick/affirmf.h>
#include <lockpick/container_of.h>

#define __LP_TEST_SLAB_ENTRIES_PER_SLAB 1000
#define __LP_TEST_SLAB_CASES_PER_TEST __LP_TEST_SLAB_ENTRIES_PER_SLAB*10


static inline bool __validate_slab(const lp_slab_t *slab, size_t expected_free_entries)
{
    if(slab->__total_free != expected_free_entries)
        return false;
    size_t free_entries = 0;
    __lp_slab_block_list_t *fb_curr = slab->__fb_head;
    if(!fb_curr)
        return free_entries == expected_free_entries;
    lp_list_t *fb_curr_l = &fb_curr->__node;
    lp_list_t *fb_head_l = fb_curr_l;
    void *prev_fb_end = NULL;
    free_entries += fb_curr->__block_size;
    while(fb_curr->__node.next != fb_head_l)
    {
        prev_fb_end = fb_curr->__base + fb_curr->__block_size*slab->__entry_size;
        fb_curr_l = fb_curr_l->next;
        fb_curr = container_of(fb_curr_l,__lp_slab_block_list_t,__node);
        if(prev_fb_end >= fb_curr->__base)
            return false;
        free_entries += fb_curr->__block_size;
    }

    return free_entries == expected_free_entries;
}


#define TEST_SLAB_RANDOM_ALLOC_FREE(entry_type)                                                                                             \
void test_slab_##entry_type##_random_alloc_free()                                                                                           \
{                                                                                                                                           \
    lp_slab_t *slab = lp_slab_create(__LP_TEST_SLAB_ENTRIES_PER_SLAB,sizeof(entry_type));                                                   \
    affirmf(slab,                                                                                                                           \
        "Failed to allocate for slab with %d entries with %ld bytes per entry",                                                             \
        __LP_TEST_SLAB_ENTRIES_PER_SLAB,sizeof(entry_type));                                                                                \
    entry_type *entries[__LP_TEST_SLAB_ENTRIES_PER_SLAB] = {NULL};                                                                          \
    entry_type preserved_entries[__LP_TEST_SLAB_ENTRIES_PER_SLAB] = {0};                                                                    \
    srand(0);                                                                                                                               \
    size_t current_free_entries = __LP_TEST_SLAB_ENTRIES_PER_SLAB;                                                                          \
    for(size_t entry_i = 0; entry_i < __LP_TEST_SLAB_ENTRIES_PER_SLAB; ++entry_i)                                                           \
    {                                                                                                                                       \
        entries[entry_i] = lp_slab_alloc(slab);                                                                                             \
        *entries[entry_i] = rand() % 256;                                                                                                   \
        preserved_entries[entry_i] = *entries[entry_i];                                                                                     \
        --current_free_entries;                                                                                                             \
        bool slab_valid = __validate_slab(slab,current_free_entries);                                                                       \
        bool entries_valid = true;                                                                                                          \
        for(size_t entry_j = 0; entry_j <= entry_i; ++entry_j)                                                                              \
            entries_valid &= *entries[entry_j] == preserved_entries[entry_j];                                                               \
        LP_TEST_ASSERT(slab_valid && entries_valid,"Slab memory inconsitency at initial population at entry %ld",entry_i);                  \
    }                                                                                                                                       \
    for(size_t i = 0; i < __LP_TEST_SLAB_ENTRIES_PER_SLAB/2; ++i)                                                                           \
    {                                                                                                                                       \
        size_t entry_i = rand() % __LP_TEST_SLAB_ENTRIES_PER_SLAB;                                                                          \
        if(entries[entry_i] == NULL)                                                                                                        \
            continue;                                                                                                                       \
        lp_slab_free(slab,entries[entry_i]);                                                                                                \
        entries[entry_i] = NULL;                                                                                                            \
        ++current_free_entries;                                                                                                             \
        bool slab_valid = __validate_slab(slab,current_free_entries);                                                                       \
        bool entries_valid = true;                                                                                                          \
        for(size_t entry_j = 0; entry_j < __LP_TEST_SLAB_ENTRIES_PER_SLAB; ++entry_j)                                                       \
            entries_valid &= !entries[entry_j] || *entries[entry_j] == preserved_entries[entry_j];                                          \
        LP_TEST_ASSERT(slab_valid && entries_valid,"Slab memory inconsitency on purge phase at entry %ld, iteration %ld",entry_i,i);        \
    }                                                                                                                                       \
    for(size_t case_i = 0; case_i < __LP_TEST_SLAB_CASES_PER_TEST; ++case_i)                                                                \
    {                                                                                                                                       \
        size_t entry_i = rand() % __LP_TEST_SLAB_ENTRIES_PER_SLAB;                                                                          \
        if(entries[entry_i])                                                                                                                \
        {                                                                                                                                   \
            lp_slab_free(slab,entries[entry_i]);                                                                                            \
            entries[entry_i] = NULL;                                                                                                        \
            ++current_free_entries;                                                                                                         \
        }                                                                                                                                   \
        else                                                                                                                                \
        {                                                                                                                                   \
            entries[entry_i] = lp_slab_alloc(slab);                                                                                         \
            *entries[entry_i] = rand() % 256;                                                                                               \
            preserved_entries[entry_i] = *entries[entry_i];                                                                                 \
            --current_free_entries;                                                                                                         \
        }                                                                                                                                   \
        bool slab_valid = __validate_slab(slab,current_free_entries);                                                                       \
        bool entries_valid = true;                                                                                                          \
        for(size_t entry_j = 0; entry_j < __LP_TEST_SLAB_ENTRIES_PER_SLAB; ++entry_j)                                                       \
            entries_valid &= !entries[entry_j] || *entries[entry_j] == preserved_entries[entry_j];                                          \
        LP_TEST_ASSERT(slab_valid && entries_valid,"Slab memory inconsitency on random phase at entry %ld in case %ld",entry_i,case_i);     \
    }                                                                                                                                       \
    for(size_t entry_i = 0; entry_i < __LP_TEST_SLAB_ENTRIES_PER_SLAB; ++entry_i)                                                           \
    {                                                                                                                                       \
        if(!entries[entry_i])                                                                                                               \
            continue;                                                                                                                       \
        lp_slab_free(slab,entries[entry_i]);                                                                                                \
        entries[entry_i] = NULL;                                                                                                            \
        ++current_free_entries;                                                                                                             \
        bool slab_valid = __validate_slab(slab,current_free_entries);                                                                       \
        bool entries_valid = true;                                                                                                          \
        for(size_t entry_j = 0; entry_j < __LP_TEST_SLAB_ENTRIES_PER_SLAB; ++entry_j)                                                       \
            entries_valid &= !entries[entry_j] || *entries[entry_j] == preserved_entries[entry_j];                                          \
        LP_TEST_ASSERT(slab_valid && entries_valid,"Slab memory inconsitency on clean-up phase at entry %ld",entry_i);                      \
    }                                                                                                                                       \
    lp_test_cleanup:                                                                                                                        \
    lp_slab_release(slab);                                                                                                                  \
}


TEST_SLAB_RANDOM_ALLOC_FREE(uint64_t)
TEST_SLAB_RANDOM_ALLOC_FREE(uint32_t)
TEST_SLAB_RANDOM_ALLOC_FREE(uint16_t)
TEST_SLAB_RANDOM_ALLOC_FREE(uint8_t)


void lp_test_slab()
{
    LP_TEST_RUN(test_slab_uint64_t_random_alloc_free());
    LP_TEST_RUN(test_slab_uint32_t_random_alloc_free());
    LP_TEST_RUN(test_slab_uint16_t_random_alloc_free());
    LP_TEST_RUN(test_slab_uint8_t_random_alloc_free());
}