#include <lockpick/vector.h>
#include <lockpick/affirmf.h>
#include <malloc.h>
#include <string.h>


lp_vector_t *lp_vector_create(size_t capacity, size_t entry_size)
{
    capacity = MAX(capacity,__LP_VECTOR_MIN_CAPACITY);

    size_t vec_size = sizeof(lp_vector_t);
    lp_vector_t *vec = (lp_vector_t*)malloc(vec_size);
    affirm_bad_malloc(vec,"vector struct",vec_size);

    vec->__buffer = calloc(capacity,entry_size);
    affirm_bad_malloc(vec->__buffer,"vector buffer",capacity*entry_size);

    vec->size = 0;
    vec->capacity = capacity;
    vec->entry_size = entry_size;

    return vec;
}


void lp_vector_release(lp_vector_t *vec)
{
    affirm_nullptr(vec,"vector");

    free(vec->__buffer);
    free(vec);
}


static inline void *__lp_vector_at(const lp_vector_t *vec, size_t i)
{
    size_t offset = i*vec->entry_size;
    return vec->__buffer+offset;
}


inline void *lp_vector_at(const lp_vector_t *vec, size_t i)
{
    affirm_nullptr(vec,"vector");

    return __lp_vector_at(vec,i);
}


static inline void __lp_vector_change_capacity(lp_vector_t *vec, size_t new_capacity)
{
    size_t new_buffer_size = new_capacity*sizeof(vec->entry_size);
    vec->__buffer = realloc(vec->__buffer,new_buffer_size);
    affirm_bad_malloc(vec->__buffer,"vector buffer realloc",new_buffer_size);

    vec->capacity = new_capacity;
}


void lp_vector_push_back(lp_vector_t *vec, void *entry)
{
    affirm_nullptr(vec,"vector");
    affirm_nullptr(entry,"entry");

    if(vec->size == vec->capacity)
    {
        size_t new_capacity = __LP_VECTOR_GROW_FACTOR*vec->capacity;
        __lp_vector_change_capacity(vec,new_capacity);
    }
    
    memcpy(__lp_vector_at(vec,vec->size),entry,vec->entry_size);
    ++vec->size;
}


static inline bool __lp_vector_require_truncate(lp_vector_t *vec)
{
    size_t effective_size = MAX(vec->size,__LP_VECTOR_MIN_CAPACITY);
    return effective_size*__LP_VECTOR_REDUCE_THR_LOAD < vec->capacity;
}


void lp_vector_pop_back(lp_vector_t *vec)
{
    affirm_nullptr(vec,"vector");
    affirmf(vec->size > 0,"Vector is empty");

    --vec->size;

    if(__lp_vector_require_truncate(vec))
    {
        size_t new_capacity = vec->size*__LP_VECTOR_REDUCE_FACTOR;
        __lp_vector_change_capacity(vec,new_capacity);
    }
}


void *lp_vector_back(const lp_vector_t *vec)
{
    affirm_nullptr(vec,"vector");
    affirmf(vec->size > 0,"Vector is empty");

    return __lp_vector_at(vec,vec->size-1);
}


void lp_vector_clear(lp_vector_t *vec)
{
    affirm_nullptr(vec,"vector");
    
    vec->size = 0;

    __lp_vector_change_capacity(vec,__LP_VECTOR_MIN_CAPACITY);
}


void lp_vector_reserve(lp_vector_t *vec, size_t new_capacity)
{
    affirm_nullptr(vec,"vector");
    affirmf(new_capacity > 0,"New capacity must be greater than zero, got: %zd",new_capacity);
    affirmf(vec->size <= new_capacity,
        "New capacity (%zd) must be greater than current vector size (%zd)",
        new_capacity,vec->size);
    
    __lp_vector_change_capacity(vec,new_capacity);
}


bool lp_vector_empty(const lp_vector_t *vec)
{
    affirm_nullptr(vec,"vector");

    return vec->size == 0;
}


void lp_vector_remove_i(lp_vector_t *vec, size_t index)
{
    affirm_nullptr(vec,"vector");
    affirmf(index < vec->size,"Index must be less than vector size, got: %zd",index);

    for(size_t i = index+1; i < vec->size; ++i)
    {
        void *prev_el = vec->__buffer+(i-1)*vec->entry_size;
        void *curr_el = prev_el+vec->entry_size;
        memcpy(prev_el,curr_el,vec->entry_size);
    }

    --vec->size;

    if(__lp_vector_require_truncate(vec))
    {
        size_t new_capacity = vec->size*__LP_VECTOR_REDUCE_FACTOR;
        __lp_vector_change_capacity(vec,new_capacity);
    }
}
