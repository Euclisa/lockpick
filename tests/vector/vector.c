#include <lockpick/test.h>
#include <lockpick/vector.h>

#define LP_TEST_VECTOR_ELEMENTS_NUM 255


#define LP_TEST_VECTOR_UINT(uint_t,test_elements_num)                                                   \
void test_vector_push_pop_##uint_t()                                                                    \
{                                                                                                       \
    lp_vector_t *vec = lp_vector_create(0,sizeof(uint_t));                                              \
    for(uint_t i = 0; i < test_elements_num; ++i)                                                       \
    {                                                                                                   \
        lp_vector_push_back(vec,&i);                                                                    \
        uint_t *obt_back = lp_vector_back(vec);                                                         \
        LP_TEST_ASSERT(*obt_back == i,"Expected back: %ld, got: %ld",(long)i,(long)*obt_back);          \
    }                                                                                                   \
    for(uint_t i = 0; i < test_elements_num; ++i)                                                       \
    {                                                                                                   \
        uint_t *obt_i = lp_vector_at(vec,i);                                                            \
        LP_TEST_ASSERT(*obt_i == i,"Expected: %d, got: %d",(long)i,(long)*obt_i);                       \
    }                                                                                                   \
    for(uint_t i = 0; i < test_elements_num; ++i)                                                       \
        lp_vector_pop_back(vec);                                                                        \
    LP_TEST_ASSERT(lp_vector_empty(vec),"Vector must be empty");                                        \
    lp_test_cleanup:                                                                                    \
    lp_vector_release(vec);                                                                             \
}                                                                                                       \
void test_vector_reserved_push_pop_##uint_t()                                                           \
{                                                                                                       \
    lp_vector_t *vec = lp_vector_create(0,sizeof(uint_t));                                              \
    size_t init_capacity = test_elements_num/2;                                                         \
    lp_vector_reserve(vec,init_capacity);                                                               \
    LP_TEST_ASSERT(vec->capacity == init_capacity,                                                      \
        "Expected init capacity: %zd, got: %zd",init_capacity,vec->capacity);                           \
    for(uint_t i = 0; i < test_elements_num; ++i)                                                       \
    {                                                                                                   \
        lp_vector_push_back(vec,&i);                                                                    \
        uint_t *obt_back = lp_vector_back(vec);                                                         \
        LP_TEST_ASSERT(*obt_back == i,"Expected back: %ld, got: %ld",(long)i,(long)*obt_back);          \
    }                                                                                                   \
    for(uint_t i = 0; i < test_elements_num; ++i)                                                       \
    {                                                                                                   \
        uint_t *obt_i = lp_vector_at(vec,i);                                                            \
        LP_TEST_ASSERT(*obt_i == i,"Expected: %ld, got: %ld",(long)i,(long)*obt_i);                     \
    }                                                                                                   \
    for(uint_t i = 0; i < test_elements_num; ++i)                                                       \
        lp_vector_pop_back(vec);                                                                        \
    LP_TEST_ASSERT(lp_vector_empty(vec),"Vector must be empty");                                        \
    lp_test_cleanup:                                                                                    \
    lp_vector_release(vec);                                                                             \
}                                                                                                       \
void test_vector_reserved_push_clear_##uint_t()                                                         \
{                                                                                                       \
    lp_vector_t *vec = lp_vector_create(0,sizeof(uint_t));                                              \
    size_t init_capacity = test_elements_num;                                                           \
    lp_vector_reserve(vec,init_capacity);                                                               \
    LP_TEST_ASSERT(vec->capacity == init_capacity,                                                      \
        "Expected init capacity: %zd, got: %zd",init_capacity,vec->capacity);                           \
    for(uint_t i = 0; i < test_elements_num; ++i)                                                       \
    {                                                                                                   \
        lp_vector_push_back(vec,&i);                                                                    \
        uint_t *obt_back = lp_vector_back(vec);                                                         \
        LP_TEST_ASSERT(*obt_back == i,"Expected back: %ld, got: %ld",(long)i,(long)*obt_back);          \
    }                                                                                                   \
    for(uint_t i = 0; i < test_elements_num; ++i)                                                       \
    {                                                                                                   \
        uint_t *obt_i = lp_vector_at(vec,i);                                                            \
        LP_TEST_ASSERT(*obt_i == i,"Expected: %ld, got: %ld",(long)i,(long)*obt_i);                     \
    }                                                                                                   \
    lp_vector_clear(vec);                                                                               \
    LP_TEST_ASSERT(lp_vector_empty(vec),"Vector must be empty");                                        \
    LP_TEST_ASSERT(vec->capacity == __LP_VECTOR_MIN_CAPACITY,                                           \
        "Expected capacity to be set to minimum value after clear, got: %zd",vec->capacity);            \
    lp_test_cleanup:                                                                                    \
    lp_vector_release(vec);                                                                             \
}                                                                                                       \
void test_vector_remove_i_##uint_t()                                                                    \
{                                                                                                       \
    lp_vector_t *vec = lp_vector_create(0,sizeof(uint_t));                                              \
    for(uint_t i = 0; i < test_elements_num; ++i)                                                       \
        lp_vector_push_back(vec,&i);                                                                    \
    size_t middle_i = test_elements_num/2;                                                              \
    while(vec->size > middle_i)                                                                         \
    {                                                                                                   \
        uint_t obt_back = lp_vector_back_type(vec,uint_t);                                              \
        uint_t true_back = test_elements_num-1;                                                         \
        LP_TEST_ASSERT(obt_back == true_back,                                                           \
            "Expected: %ld, got: %ld during middle removal",(long)true_back,(long)obt_back);            \
        lp_vector_remove_i(vec,middle_i);                                                               \
    }                                                                                                   \
    for(uint_t i = 0; i < middle_i; ++i)                                                                \
    {                                                                                                   \
        uint_t obt_i = lp_vector_at_type(vec,i,uint_t);                                                 \
        LP_TEST_ASSERT(obt_i == i,                                                                      \
            "Expected: %ld, got: %ld, after removing last halve",                                       \
            (long)i,(long)obt_i);                                                                       \
    }                                                                                                   \
    for(uint_t i = 0; i < middle_i; ++i)                                                                \
    {                                                                                                   \
        lp_vector_remove_i(vec,0);                                                                      \
        for(uint_t j = 0; j < middle_i-i-1; ++j)                                                        \
        {                                                                                               \
            uint_t true_i = j+i+1;                                                                      \
            uint_t obt_i = lp_vector_at_type(vec,j,uint_t);                                             \
            LP_TEST_ASSERT(true_i == obt_i,                                                             \
                "Expected: %ld, got: %ld on base removal",true_i,obt_i);                                \
        }                                                                                               \
    }                                                                                                   \
    lp_test_cleanup:                                                                                    \
    lp_vector_release(vec);                                                                             \
}


LP_TEST_VECTOR_UINT(uint8_t,255)
LP_TEST_VECTOR_UINT(uint16_t,1000)
LP_TEST_VECTOR_UINT(uint32_t,1000)
LP_TEST_VECTOR_UINT(uint64_t,1000)


void lp_test_vector()
{
    LP_TEST_RUN(test_vector_push_pop_uint8_t());
    LP_TEST_RUN(test_vector_reserved_push_pop_uint8_t());
    LP_TEST_RUN(test_vector_reserved_push_clear_uint8_t());
    LP_TEST_RUN(test_vector_remove_i_uint8_t());

    LP_TEST_RUN(test_vector_push_pop_uint16_t());
    LP_TEST_RUN(test_vector_reserved_push_pop_uint16_t());
    LP_TEST_RUN(test_vector_reserved_push_clear_uint16_t());
    LP_TEST_RUN(test_vector_remove_i_uint16_t());

    LP_TEST_RUN(test_vector_push_pop_uint32_t());
    LP_TEST_RUN(test_vector_reserved_push_pop_uint32_t());
    LP_TEST_RUN(test_vector_reserved_push_clear_uint32_t());
    LP_TEST_RUN(test_vector_remove_i_uint32_t());

    LP_TEST_RUN(test_vector_push_pop_uint64_t());
    LP_TEST_RUN(test_vector_reserved_push_pop_uint64_t());
    LP_TEST_RUN(test_vector_reserved_push_clear_uint64_t());
    LP_TEST_RUN(test_vector_remove_i_uint64_t());
}