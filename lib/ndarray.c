#include <lockpick/ndarray.h>
#include <lockpick/affirmf.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>


lp_ndarray_t *lp_ndarray_create(size_t total_size, size_t entry_size, size_t *shape, uint32_t dims)
{
    size_t ndarr_size = sizeof(lp_ndarray_t);
    lp_ndarray_t *ndarr = (lp_ndarray_t*)malloc(ndarr_size);
    affirm_bad_malloc(ndarr,"ndarray",ndarr_size);

    size_t buffer_size = total_size*entry_size;
    ndarr->__buffer = malloc(buffer_size);
    affirm_bad_malloc(ndarr->__buffer,"ndarray buffer",buffer_size);

    if(shape)
    {
        affirmf(dims > 0,"Shape tuple is not null, but zero dimensions specified");
        size_t shape_size = sizeof(size_t)*dims;
        ndarr->shape = (size_t*)malloc(shape_size);
        affirm_bad_malloc(ndarr->shape,"shape tuple",shape_size);
        memcpy(ndarr->shape,shape,shape_size);
        ndarr->dims = dims;
    }
    else
    {
        size_t shape_size = sizeof(size_t);
        ndarr->shape = (size_t*)malloc(shape_size);
        ndarr->shape[0] = total_size;
        ndarr->dims = 1;
    }

    ndarr->entry_size = entry_size;
    ndarr->size = total_size;

    return ndarr;
}


void lp_ndarray_reshape(lp_ndarray_t *ndarr, const size_t *new_shape, uint32_t new_dims)
{
    affirm_nullptr(ndarr,"ndarray");
    affirm_nullptr(new_shape,"new shape tuple");
    affirmf(new_dims > 0,"Number of new dimensions must be greater than zero");

    size_t new_size = 1;
    for(uint32_t dim_i = 0; dim_i < new_dims; ++dim_i)
        new_size *= new_shape[dim_i];
    affirmf(new_size == ndarr->size,"New shape must contain %zd elements, but got shape of %zd",ndarr->size,new_size);

    size_t new_shape_size = new_dims*sizeof(size_t);
    if(new_dims != ndarr->dims)
    {
        ndarr->shape = realloc(ndarr->shape,new_shape_size);
        affirm_bad_malloc(ndarr->shape,"new shape tuple",new_shape_size);
        ndarr->dims = new_dims;
    }
    memcpy(ndarr->shape,new_shape,new_shape_size);
}


void *lp_ndarray_at(const lp_ndarray_t *ndarr, const size_t *inds)
{
    affirm_nullptr(ndarr,"ndarray");
    affirm_nullptr(inds,"indices");

    size_t i = 0;
    size_t gap = 1;
    for(int64_t dim_i = ndarr->dims-1; dim_i >= 0; --dim_i)
    {
        i += gap*inds[dim_i];
        gap *= ndarr->shape[dim_i];
    }

    return ndarr->__buffer + i*ndarr->entry_size;
}


void lp_ndarray_release(lp_ndarray_t *ndarr)
{
    affirm_nullptr(ndarr,"ndarray");

    free(ndarr->__buffer);
    free(ndarr->shape);
    free(ndarr);
}

