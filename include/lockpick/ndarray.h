#ifndef _LOCKPICK_NDARRAY_H
#define _LOCKPICK_NDARRAY_H

#include <stdint.h>
#include <stddef.h>


typedef struct lp_ndarray
{
    void *__buffer;
    size_t entry_size;
    size_t size;
    size_t *shape;
    uint32_t dims;
} lp_ndarray_t;


lp_ndarray_t *lp_ndarray_create(size_t total_size, size_t entry_size, size_t *shape, uint32_t dims);

void lp_ndarray_reshape(lp_ndarray_t *ndarr, const size_t *new_shape, uint32_t new_dims);

void *lp_ndarray_at(const lp_ndarray_t *ndarr, const size_t *inds);

void lp_ndarray_release(lp_ndarray_t *ndarr);

#endif // _LOCKPICK_NDARRAY_H