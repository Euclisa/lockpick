#ifndef _LOCKPICK_VECTOR_H
#define _LOCKPICK_VECTOR_H

#include <stddef.h>
#include <stdbool.h>

#define __LP_VECTOR_GROW_FACTOR 2
#define __LP_VECTOR_REDUCE_FACTOR 2
#define __LP_VECTOR_REDUCE_THR_LOAD 4
#define __LP_VECTOR_MIN_CAPACITY 1


typedef struct lp_vector
{
    void *__buffer;
    size_t size;
    size_t capacity;
    size_t entry_size;
} lp_vector_t;


lp_vector_t *lp_vector_create(size_t capacity, size_t entry_size);

void lp_vector_release(lp_vector_t *vec);

void *lp_vector_at(const lp_vector_t *vec, size_t i);
#define lp_vector_at_type(vec,i,type) *((type*)lp_vector_at(vec,i))

void lp_vector_push_back(lp_vector_t *vec, void *entry);
void lp_vector_pop_back(lp_vector_t *vec);

void *lp_vector_back(const lp_vector_t *vec);
#define lp_vector_back_type(vec,type) *((type*)lp_vector_back(vec))

void lp_vector_clear(lp_vector_t *vec);

void lp_vector_reserve(lp_vector_t *vec, size_t new_capacity);

bool lp_vector_empty(const lp_vector_t *vec);

void lp_vector_remove_i(lp_vector_t *vec, size_t index);

#endif // _LOCKPICK_VECTOR_H