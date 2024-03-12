#include <lockpick/test.h>
#include <lockpick/ndarray.h>
#include <lockpick/affirmf.h>

#define LP_TEST_NDARRAY_UINT(uint_t)                                                                                                      \
void lp_test_ndarray_1d_populate_##uint_t(uint32_t ndarr_size)                                                                            \
{                                                                                                                                         \
    lp_ndarray_t *ndarr = lp_ndarray_create(ndarr_size,sizeof(uint_t),NULL,1);                                                            \
    for(uint32_t i = 0; i < ndarr_size; ++i)                                                                                              \
    {                                                                                                                                     \
        size_t inds = i;                                                                                                                  \
        *((uint_t*)lp_ndarray_at(ndarr,&inds)) = i;                                                                                       \
    }                                                                                                                                     \
    for(uint32_t i = 0; i < ndarr_size; ++i)                                                                                              \
    {                                                                                                                                     \
        size_t inds = i;                                                                                                                  \
        uint32_t ndarr_val = *((uint_t*)lp_ndarray_at(ndarr,&inds));                                                                      \
        LP_TEST_ASSERT(i == ndarr_val,"Expected: %d, got: %d at index %d",i,ndarr_val,i);                                                 \
    }                                                                                                                                     \
    lp_test_cleanup:                                                                                                                      \
    lp_ndarray_release(ndarr);                                                                                                            \
}                                                                                                                                         \
void lp_test_ndarray_2d_populate_##uint_t(uint32_t ndarr_size, size_t s1, size_t s2)                                                      \
{                                                                                                                                         \
    affirmf(ndarr_size == s1*s2, "Shape (%zd, %zd) is inconsistent with size %d",s1,s2,ndarr_size);                                       \
    size_t shape[] = {s1, s2};                                                                                                            \
    lp_ndarray_t *ndarr = lp_ndarray_create(ndarr_size,sizeof(uint_t),shape,2);                                                           \
    for(uint32_t i = 0; i < shape[0]; ++i)                                                                                                \
    {                                                                                                                                     \
        for(uint32_t j = 0; j < shape[1]; ++j)                                                                                            \
        {                                                                                                                                 \
            size_t inds[] = {i,j};                                                                                                        \
            *((uint_t*)lp_ndarray_at(ndarr,inds)) = (uint_t)(i+j*2);                                                                      \
        }                                                                                                                                 \
    }                                                                                                                                     \
    for(uint32_t i = 0; i < shape[0]; ++i)                                                                                                \
    {                                                                                                                                     \
        for(uint32_t j = 0; j < shape[1]; ++j)                                                                                            \
        {                                                                                                                                 \
            size_t inds[] = {i,j};                                                                                                        \
            uint32_t ndarr_val = *((uint_t*)lp_ndarray_at(ndarr,inds));                                                                   \
            uint32_t true_val = (uint_t)(i+j*2);                                                                                          \
            LP_TEST_ASSERT(true_val == ndarr_val,"Expected: %d, got: %d at position (%d, %d)",true_val,ndarr_val,i,j);                    \
        }                                                                                                                                 \
    }                                                                                                                                     \
    lp_test_cleanup:                                                                                                                      \
    lp_ndarray_release(ndarr);                                                                                                            \
}                                                                                                                                         \
void lp_test_ndarray_3d_populate_##uint_t(uint32_t ndarr_size, size_t s1, size_t s2, size_t s3)                                           \
{                                                                                                                                         \
    affirmf(ndarr_size == s1*s2*s3, "Shape (%zd, %zd, %zd) is inconsistent with size %d",s1,s2,s3,ndarr_size);                            \
    size_t shape[] = {s1, s2, s3};                                                                                                        \
    lp_ndarray_t *ndarr = lp_ndarray_create(ndarr_size,sizeof(uint_t),shape,3);                                                           \
    for(uint32_t i = 0; i < shape[0]; ++i)                                                                                                \
    {                                                                                                                                     \
        for(uint32_t j = 0; j < shape[1]; ++j)                                                                                            \
        {                                                                                                                                 \
            for(uint32_t k = 0; k < shape[2]; ++k)                                                                                        \
            {                                                                                                                             \
                size_t inds[] = {i,j,k};                                                                                                  \
                *((uint_t*)lp_ndarray_at(ndarr,inds)) = (uint_t)(i+j*2+k*3);                                                              \
            }                                                                                                                             \
        }                                                                                                                                 \
    }                                                                                                                                     \
    for(uint32_t i = 0; i < shape[0]; ++i)                                                                                                \
    {                                                                                                                                     \
        for(uint32_t j = 0; j < shape[1]; ++j)                                                                                            \
        {                                                                                                                                 \
            for(uint32_t k = 0; k < shape[2]; ++k)                                                                                        \
            {                                                                                                                             \
                size_t inds[] = {i,j,k};                                                                                                  \
                uint32_t ndarr_val = *((uint_t*)lp_ndarray_at(ndarr,inds));                                                               \
                uint32_t true_val = (uint_t)(i+j*2+k*3);                                                                                  \
                LP_TEST_ASSERT(true_val == ndarr_val,"Expected: %d, got: %d at position (%d, %d, %d)",true_val,ndarr_val,i,j,k);          \
            }                                                                                                                             \
        }                                                                                                                                 \
    }                                                                                                                                     \
    lp_test_cleanup:                                                                                                                      \
    lp_ndarray_release(ndarr);                                                                                                            \
}                                                                                                                                         \
void lp_test_ndarray_1d_2d_reshape_##uint_t(uint32_t ndarr_size, size_t new_s1, size_t new_s2)                                            \
{                                                                                                                                         \
    lp_ndarray_t *ndarr = lp_ndarray_create(ndarr_size,sizeof(uint_t),NULL,1);                                                            \
    for(uint32_t i = 0; i < ndarr_size; ++i)                                                                                              \
    {                                                                                                                                     \
        size_t inds = i;                                                                                                                  \
        *((uint_t*)lp_ndarray_at(ndarr,&inds)) = i;                                                                                       \
    }                                                                                                                                     \
    size_t new_shape[] = {new_s1, new_s2};                                                                                                \
    lp_ndarray_reshape(ndarr,new_shape,2);                                                                                                \
    size_t count = 0;                                                                                                                     \
    for(uint32_t i = 0; i < new_shape[0]; ++i)                                                                                            \
    {                                                                                                                                     \
        for(uint32_t j = 0; j < new_shape[1]; ++j)                                                                                        \
        {                                                                                                                                 \
            size_t inds[] = {i,j};                                                                                                        \
            uint32_t ndarr_val = *((uint_t*)lp_ndarray_at(ndarr,inds));                                                                   \
            LP_TEST_ASSERT(count == ndarr_val,"Expected: %d, got: %d at position (%d, %d)",count,ndarr_val,i,j);                          \
            ++count;                                                                                                                      \
        }                                                                                                                                 \
    }                                                                                                                                     \
    lp_test_cleanup:                                                                                                                      \
    lp_ndarray_release(ndarr);                                                                                                            \
}                                                                                                                                         \
void lp_test_ndarray_1d_3d_reshape_##uint_t(uint32_t ndarr_size, size_t new_s1, size_t new_s2, size_t new_s3)                             \
{                                                                                                                                         \
    lp_ndarray_t *ndarr = lp_ndarray_create(ndarr_size,sizeof(uint_t),NULL,1);                                                            \
    for(uint32_t i = 0; i < ndarr_size; ++i)                                                                                              \
    {                                                                                                                                     \
        size_t inds = i;                                                                                                                  \
        *((uint_t*)lp_ndarray_at(ndarr,&inds)) = i;                                                                                       \
    }                                                                                                                                     \
    size_t new_shape[] = {new_s1, new_s2, new_s3};                                                                                        \
    lp_ndarray_reshape(ndarr,new_shape,3);                                                                                                \
    size_t count = 0;                                                                                                                     \
    for(uint32_t i = 0; i < new_shape[0]; ++i)                                                                                            \
    {                                                                                                                                     \
        for(uint32_t j = 0; j < new_shape[1]; ++j)                                                                                        \
        {                                                                                                                                 \
            for(uint32_t k = 0; k < new_shape[2]; ++k)                                                                                    \
            {                                                                                                                             \
                size_t inds[] = {i,j,k};                                                                                                  \
                uint32_t ndarr_val = *((uint_t*)lp_ndarray_at(ndarr,inds));                                                               \
                LP_TEST_ASSERT(count == ndarr_val,"Expected: %d, got: %d at position (%d, %d, %d)",count,ndarr_val,i,j,k);                \
                ++count;                                                                                                                  \
            }                                                                                                                             \
        }                                                                                                                                 \
    }                                                                                                                                     \
    lp_test_cleanup:                                                                                                                      \
    lp_ndarray_release(ndarr);                                                                                                            \
}                                                                                                                                         \
void lp_test_ndarray_##uint_t()                                                                                                           \
{                                                                                                                                         \
    LP_TEST_RUN(lp_test_ndarray_1d_populate_##uint_t(64));                                                                                \
    LP_TEST_RUN(lp_test_ndarray_1d_populate_##uint_t(128));                                                                               \
    LP_TEST_RUN(lp_test_ndarray_1d_populate_##uint_t(256));                                                                               \
    LP_TEST_RUN(lp_test_ndarray_2d_populate_##uint_t(256,1,256));                                                                         \
    LP_TEST_RUN(lp_test_ndarray_2d_populate_##uint_t(256,2,128));                                                                         \
    LP_TEST_RUN(lp_test_ndarray_2d_populate_##uint_t(256,4,64));                                                                          \
    LP_TEST_RUN(lp_test_ndarray_2d_populate_##uint_t(256,64,4));                                                                          \
    LP_TEST_RUN(lp_test_ndarray_2d_populate_##uint_t(256,128,2));                                                                         \
    LP_TEST_RUN(lp_test_ndarray_2d_populate_##uint_t(256,256,1));                                                                         \
    LP_TEST_RUN(lp_test_ndarray_3d_populate_##uint_t(256,1,1,256));                                                                       \
    LP_TEST_RUN(lp_test_ndarray_3d_populate_##uint_t(256,2,2,64));                                                                        \
    LP_TEST_RUN(lp_test_ndarray_3d_populate_##uint_t(256,4,2,32));                                                                        \
    LP_TEST_RUN(lp_test_ndarray_3d_populate_##uint_t(256,2,64,2));                                                                        \
    LP_TEST_RUN(lp_test_ndarray_3d_populate_##uint_t(256,2,128,1));                                                                       \
    LP_TEST_RUN(lp_test_ndarray_3d_populate_##uint_t(256,64,2,2));                                                                        \
    LP_TEST_RUN(lp_test_ndarray_3d_populate_##uint_t(256,16,4,4));                                                                        \
    LP_TEST_RUN(lp_test_ndarray_3d_populate_##uint_t(256,256,1,1));                                                                       \
    LP_TEST_RUN(lp_test_ndarray_1d_2d_reshape_##uint_t(256,1,256));                                                                       \
    LP_TEST_RUN(lp_test_ndarray_1d_2d_reshape_##uint_t(256,2,128));                                                                       \
    LP_TEST_RUN(lp_test_ndarray_1d_2d_reshape_##uint_t(256,4,64));                                                                        \
    LP_TEST_RUN(lp_test_ndarray_1d_2d_reshape_##uint_t(256,64,4));                                                                        \
    LP_TEST_RUN(lp_test_ndarray_1d_2d_reshape_##uint_t(256,128,2));                                                                       \
    LP_TEST_RUN(lp_test_ndarray_1d_2d_reshape_##uint_t(256,256,1));                                                                       \
    LP_TEST_RUN(lp_test_ndarray_1d_3d_reshape_##uint_t(256,1,1,256));                                                                     \
    LP_TEST_RUN(lp_test_ndarray_1d_3d_reshape_##uint_t(256,2,2,64));                                                                      \
    LP_TEST_RUN(lp_test_ndarray_1d_3d_reshape_##uint_t(256,4,2,32));                                                                      \
    LP_TEST_RUN(lp_test_ndarray_1d_3d_reshape_##uint_t(256,2,64,2));                                                                      \
    LP_TEST_RUN(lp_test_ndarray_1d_3d_reshape_##uint_t(256,2,128,1));                                                                     \
    LP_TEST_RUN(lp_test_ndarray_1d_3d_reshape_##uint_t(256,64,2,2));                                                                      \
    LP_TEST_RUN(lp_test_ndarray_1d_3d_reshape_##uint_t(256,16,4,4));                                                                      \
    LP_TEST_RUN(lp_test_ndarray_1d_3d_reshape_##uint_t(256,256,1,1));                                                                     \
}                                                                                                                                         \

LP_TEST_NDARRAY_UINT(uint8_t)
LP_TEST_NDARRAY_UINT(uint16_t)
LP_TEST_NDARRAY_UINT(uint32_t)
LP_TEST_NDARRAY_UINT(uint64_t)


void lp_test_ndarray()
{
    LP_TEST_RUN(lp_test_ndarray_uint8_t());
    LP_TEST_RUN(lp_test_ndarray_uint16_t());
    LP_TEST_RUN(lp_test_ndarray_uint32_t());
    LP_TEST_RUN(lp_test_ndarray_uint64_t());
}
