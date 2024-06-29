#include <lockpick/test.h>
#include <lockpick/set.h>
#include <lockpick/utility.h>
#include <lockpick/affirmf.h>

#define TEST_SET_ENTRIES_NUM 252

#define LP_TEST_SET_UINT(uint_t)                                                                                                                                  \
static bool less_##uint_t(const uint_t *a, const uint_t *b)                                                                                                       \
{                                                                                                                                                                 \
    return *a < *b;                                                                                                                                               \
}                                                                                                                                                                 \
void __test_set_general_##uint_t()                                                                                                                                \
{                                                                                                                                                                 \
    lp_set_t *set = lp_set_create(sizeof(uint_t),(bool (*)(const void*, const void*))less_##uint_t);                                                              \
    LP_TEST_ASSERT(lp_set_is_empty(set),"Expected newly created set to be empty");                                                                                \
    size_t set_size = lp_set_size(set);                                                                                                                           \
    LP_TEST_ASSERT(set_size == 0,"Expected newly created set to have size 0, got %zd",set_size);                                                                  \
    uint_t entry_values[TEST_SET_ENTRIES_NUM];                                                                                                                    \
    for(uint_t i = 0; i < TEST_SET_ENTRIES_NUM; ++i)                                                                                                              \
        entry_values[i] = i;                                                                                                                                      \
    lp_shuffle(entry_values,TEST_SET_ENTRIES_NUM);                                                                                                                \
    for(uint_t i = 0; i < TEST_SET_ENTRIES_NUM; ++i)                                                                                                              \
    {                                                                                                                                                             \
        bool status;                                                                                                                                              \
        const lp_set_entry_t *inserted_entry = lp_set_insert(set,&entry_values[i],&status);                                                                       \
        LP_TEST_ASSERT(status,"Failed to insert entry at index %ld",(long)i);                                                                                     \
        uint_t inserted_value = *(uint_t*)inserted_entry->data;                                                                                                   \
        LP_TEST_ASSERT(inserted_value == entry_values[i],                                                                                                         \
            "Expected inserted value to be %ld, but got %ld",(long)entry_values[i],(long)inserted_value);                                                         \
        ++set_size;                                                                                                                                               \
        size_t curr_size = lp_set_size(set);                                                                                                                      \
        LP_TEST_ASSERT(set_size == curr_size,"Expected set size to be %zd, got %zd",set_size,curr_size);                                                          \
    }                                                                                                                                                             \
    for(uint_t i = 0; i < TEST_SET_ENTRIES_NUM; ++i)                                                                                                              \
    {                                                                                                                                                             \
        bool status;                                                                                                                                              \
        const lp_set_entry_t *inserted_entry = lp_set_insert(set,&entry_values[i],&status);                                                                       \
        LP_TEST_ASSERT(!status,"Expected insertion fail on second insert of element at index %ld",(long)i);                                                       \
        uint_t inserted_value = *(uint_t*)inserted_entry->data;                                                                                                   \
        LP_TEST_ASSERT(inserted_value == entry_values[i],                                                                                                         \
            "Expected inserted value to be %ld, but got %ld",(long)entry_values[i],(long)inserted_value);                                                         \
        size_t curr_size = lp_set_size(set);                                                                                                                      \
        LP_TEST_ASSERT(set_size == curr_size,"Expected set size to remain unchanged and equal to %zd, got %zd",set_size,curr_size);                               \
    }                                                                                                                                                             \
    for(uint_t i = 0; i < TEST_SET_ENTRIES_NUM; ++i)                                                                                                              \
    {                                                                                                                                                             \
        uint_t res;                                                                                                                                               \
        bool status = lp_set_find(set,&entry_values[i],&res);                                                                                                     \
        LP_TEST_ASSERT(status,"Expected entry %ld at index %ld to be present in set, but not found",(long)entry_values[i],(long)i);                               \
        LP_TEST_ASSERT(res == entry_values[i],"Expected entry at index %ld to be equal %ld, but %ld was found",(long)i,(long)entry_values[i],(long)res);          \
    }                                                                                                                                                             \
    const lp_set_entry_t *current_entry = lp_set_begin(set);                                                                                                      \
    LP_TEST_ASSERT(current_entry,"Expected non-null begin set iterator for non-empty set (first traverse)");                                                      \
    for(uint_t i = 0; i < TEST_SET_ENTRIES_NUM; ++i)                                                                                                              \
    {                                                                                                                                                             \
        uint_t found = *(uint_t*)current_entry->data;                                                                                                             \
        LP_TEST_ASSERT(i == found,"Expected element at index %ld to be equal %ld, but found %ld (forward)",(long)i,(long)i,(long)found);                          \
        const lp_set_entry_t *prev_entry = lp_set_prev(current_entry);                                                                                            \
        if(i > 0)                                                                                                                                                 \
        {                                                                                                                                                         \
            uint_t prev_value = *(uint_t*)prev_entry->data;                                                                                                       \
            LP_TEST_ASSERT((i-1) == prev_value,"Expected previous value at index %ld to be %ld, got %ld",(long)i,(long)(i-1),(long)prev_value);                   \
        }                                                                                                                                                         \
        else                                                                                                                                                      \
            LP_TEST_ASSERT(prev_entry == NULL,"Expected previous entry of the first value to be equal to NULL");                                                  \
        current_entry = lp_set_next(current_entry);                                                                                                               \
    }                                                                                                                                                             \
    current_entry = lp_set_end(set);                                                                                                                              \
    for(long i = TEST_SET_ENTRIES_NUM-1; i >= 0; --i)                                                                                                             \
    {                                                                                                                                                             \
        uint_t found = *(uint_t*)current_entry->data;                                                                                                             \
        LP_TEST_ASSERT(i == found,"Expected element at index %ld to be equal %ld, but found %ld (backward)",(long)i,(long)i,(long)found);                         \
        const lp_set_entry_t *next_entry = lp_set_next(current_entry);                                                                                            \
        if(i < TEST_SET_ENTRIES_NUM-1)                                                                                                                            \
        {                                                                                                                                                         \
            uint_t next_value = *(uint_t*)next_entry->data;                                                                                                       \
            LP_TEST_ASSERT((i+1) == next_value,"Expected next value at index %ld to be %ld, got %ld",(long)i,(long)(i+1),(long)next_value);                       \
        }                                                                                                                                                         \
        else                                                                                                                                                      \
            LP_TEST_ASSERT(next_entry == NULL,"Expected next entry of the last value to be equal to NULL");                                                       \
        current_entry = lp_set_prev(current_entry);                                                                                                               \
    }                                                                                                                                                             \
    LP_TEST_ASSERT(current_entry == NULL,"Expected NULL entry to be returned from 'lp_set_next' after all entries have been visited");                            \
    for(uint_t i = 0; i < TEST_SET_ENTRIES_NUM; i += 2)                                                                                                           \
    {                                                                                                                                                             \
        LP_TEST_ASSERT(lp_set_remove(set,&i),"Failed to remove entry at index %ld",(long)i);                                                                      \
        bool find_status = lp_set_find(set,&i,NULL);                                                                                                              \
        LP_TEST_ASSERT(!find_status,"Found removed element %ld",(long)i);                                                                                         \
        --set_size;                                                                                                                                               \
        size_t curr_size = lp_set_size(set);                                                                                                                      \
        LP_TEST_ASSERT(set_size == curr_size,"Expected set size to be %zd, got %zd",set_size,curr_size);                                                          \
    }                                                                                                                                                             \
    current_entry = lp_set_begin(set);                                                                                                                            \
    LP_TEST_ASSERT(current_entry,"Expected non-null begin set iterator for non-empty set (second traverse)");                                                     \
    for(uint_t i = 1; i < TEST_SET_ENTRIES_NUM; i += 2)                                                                                                           \
    {                                                                                                                                                             \
        uint_t found = *(uint_t*)current_entry->data;                                                                                                           \
        LP_TEST_ASSERT(i == found,"Expected element at index %ld to be equal %ld, but found %ld",(long)i,(long)i,(long)found);                                    \
        current_entry = lp_set_next(current_entry);                                                                                                               \
    }                                                                                                                                                             \
    for(uint_t i = 1; i < TEST_SET_ENTRIES_NUM; i += 2)                                                                                                           \
    {                                                                                                                                                             \
        LP_TEST_ASSERT(lp_set_remove(set,&i),"Failed to remove entry at index %ld",(long)i);                                                                      \
        bool find_status = lp_set_find(set,&i,NULL);                                                                                                              \
        LP_TEST_ASSERT(!find_status,"Found removed element %ld",(long)i);                                                                                         \
        --set_size;                                                                                                                                               \
        size_t curr_size = lp_set_size(set);                                                                                                                      \
        LP_TEST_ASSERT(set_size == curr_size,"Expected set size to be %zd, got %zd",set_size,curr_size);                                                          \
    }                                                                                                                                                             \
    LP_TEST_ASSERT(lp_set_is_empty(set),"Expected set to be empty after all elements removed");                                                                   \
    lp_test_cleanup:                                                                                                                                              \
    lp_set_release(set);                                                                                                                                          \
}                                                                                                                                                                 \
void test_set_general_##uint_t(size_t tests_num)                                                                                                                  \
{                                                                                                                                                                 \
    for(size_t test_i = 0; test_i < tests_num; ++test_i)                                                                                                          \
        LP_TEST_STEP_INTO(__test_set_general_##uint_t());                                                                                                         \
    lp_test_cleanup:                                                                                                                                              \
}


LP_TEST_SET_UINT(uint8_t)
LP_TEST_SET_UINT(uint16_t)
LP_TEST_SET_UINT(uint32_t)
LP_TEST_SET_UINT(uint64_t)


void lp_test_set()
{
    LP_TEST_RUN(test_set_general_uint8_t(100));
    LP_TEST_RUN(test_set_general_uint16_t(100));
    LP_TEST_RUN(test_set_general_uint32_t(100));
    LP_TEST_RUN(test_set_general_uint64_t(100));
}
