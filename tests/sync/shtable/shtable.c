#include <lockpick/test.h>
#include <lockpick/sync/shtable.h>
#include <lockpick/affirmf.h>
#include <lockpick/utility.h>
#include <pthread.h>
#include <stdlib.h>


size_t __u32_hsh(const void *x)
{
    size_t ux = *(uint32_t*)x;
    return lp_uni_hash(ux);
}


bool __u32_eq(const void *a, const void *b)
{
    size_t ua = *(uint32_t*)a;
    size_t ub = *(uint32_t*)b;
    return ua == ub;
}


enum __ht_op_type
{
    __HT_OP_INSERT,
    __HT_OP_REMOVE,
    __HT_OP_FIND
};

typedef struct __ht_op_arg
{
    lp_shtable_t *ht;
    enum __ht_op_type type;
    uint32_t value;
} __ht_op_arg_t;

void *__ht_op(void *_arg)
{
    __ht_op_arg_t *arg = (__ht_op_arg_t*)_arg;
    lp_shtable_t *ht = arg->ht;
    enum __ht_op_type type = arg->type;
    uint32_t value = arg->value;
    bool *status = (bool*)malloc(sizeof(bool));
    
    if(type == __HT_OP_INSERT)
        *status = lp_shtable_insert(ht,&value);
    else if(type == __HT_OP_REMOVE)
        *status = lp_shtable_remove(ht,&value);
    else if(type == __HT_OP_FIND)
    {
        uint32_t found;
        *status = lp_shtable_find(ht,&value,&found);
        if(*status)
            *status = found == value;
    }

    return status;
}


void __test_shtable_random_ops(size_t threads_num)
{
    lp_shtable_t *ht = lp_shtable_create(sizeof(uint32_t),__u32_hsh,__u32_eq);
    
    pthread_t *threads1 = (pthread_t*)malloc(threads_num*sizeof(pthread_t));
    __ht_op_arg_t *args1 = (__ht_op_arg_t*)malloc(threads_num*sizeof(__ht_op_arg_t));
    pthread_t *threads2 = (pthread_t*)malloc(threads_num*sizeof(pthread_t));
    __ht_op_arg_t *args2 = (__ht_op_arg_t*)malloc(threads_num*sizeof(__ht_op_arg_t));
    
    bool test_status = true;
    size_t failed_thr_i = 0;
    uint32_t failed_group_i = 0;

    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        args1[thr_i].ht = ht;
        args1[thr_i].type = __HT_OP_INSERT;
        args1[thr_i].value = thr_i;
        pthread_create(&threads1[thr_i],NULL,__ht_op,&args1[thr_i]);
    }
    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        bool *status;
        pthread_join(threads1[thr_i],(void**)&status);
        test_status &= *status;
        if(!test_status)
            failed_thr_i = thr_i;
        free(status);
    }
    LP_TEST_ASSERT(test_status,"Failed to insert entry in thread %ld of group 1. Init population",failed_thr_i);

    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        args1[thr_i].ht = ht;
        args1[thr_i].type = __HT_OP_FIND;
        args1[thr_i].value = thr_i;
        pthread_create(&threads1[thr_i],NULL,__ht_op,&args1[thr_i]);
    }
    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        bool *status;
        pthread_join(threads1[thr_i],(void**)&status);
        test_status &= *status;
        if(!test_status)
            failed_thr_i = thr_i;
        free(status);
    }
    LP_TEST_ASSERT(test_status,"Entry not found in thread %ld of group 1. Init population scan.",failed_thr_i);

    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        args1[thr_i].ht = ht;
        args1[thr_i].type = __HT_OP_INSERT;
        args1[thr_i].value = thr_i;
        pthread_create(&threads1[thr_i],NULL,__ht_op,&args1[thr_i]);
    }
    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        bool *status;
        pthread_join(threads1[thr_i],(void**)&status);
        test_status &= !*status;
        if(!test_status)
            failed_thr_i = thr_i;
        free(status);
    }
    LP_TEST_ASSERT(test_status,"Entry not found in thread %ld of group 1. Init population repeated insert.",failed_thr_i);

    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        args1[thr_i].ht = ht;
        args1[thr_i].type = __HT_OP_INSERT;
        args1[thr_i].value = thr_i+threads_num;
        pthread_create(&threads1[thr_i],NULL,__ht_op,&args1[thr_i]);

        args2[thr_i].ht = ht;
        args2[thr_i].type = __HT_OP_REMOVE;
        args2[thr_i].value = thr_i;
        pthread_create(&threads2[thr_i],NULL,__ht_op,&args2[thr_i]);
    }
    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        bool *status;
        pthread_join(threads1[thr_i],(void**)&status);
        test_status &= *status;
        if(!test_status)
        {
            failed_thr_i = thr_i;
            failed_group_i = 1;
        }
        free(status);

        pthread_join(threads2[thr_i],(void**)&status);
        test_status &= *status;
        if(!test_status)
        {
            failed_thr_i = thr_i;
            failed_group_i = 2;
        }
        free(status);
    }
    LP_TEST_ASSERT(test_status,"Failed to perform operation in thread %ld of %d group. Concurrent remove/insert.",failed_thr_i,failed_group_i);

    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        args1[thr_i].ht = ht;
        args1[thr_i].type = __HT_OP_FIND;
        args1[thr_i].value = thr_i+threads_num;
        pthread_create(&threads1[thr_i],NULL,__ht_op,&args1[thr_i]);

        args2[thr_i].ht = ht;
        args2[thr_i].type = __HT_OP_INSERT;
        args2[thr_i].value = thr_i;
        pthread_create(&threads2[thr_i],NULL,__ht_op,&args2[thr_i]);
    }
    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        bool *status;
        pthread_join(threads1[thr_i],(void**)&status);
        test_status &= *status;
        if(!test_status)
        {
            failed_thr_i = thr_i;
            failed_group_i = 1;
        }
        free(status);

        pthread_join(threads2[thr_i],(void**)&status);
        test_status &= *status;
        if(!test_status)
        {
            failed_thr_i = thr_i;
            failed_group_i = 2;
        }
        free(status);
    }
    LP_TEST_ASSERT(test_status,"Failed to perform operation in thread %ld of %d group. Concurrent find/insert.",failed_thr_i,failed_group_i);

    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        args1[thr_i].ht = ht;
        args1[thr_i].type = __HT_OP_FIND;
        args1[thr_i].value = thr_i;
        pthread_create(&threads1[thr_i],NULL,__ht_op,&args1[thr_i]);

        args2[thr_i].ht = ht;
        args2[thr_i].type = __HT_OP_REMOVE;
        args2[thr_i].value = thr_i+threads_num;
        pthread_create(&threads2[thr_i],NULL,__ht_op,&args2[thr_i]);
    }
    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        bool *status;
        pthread_join(threads1[thr_i],(void**)&status);
        test_status &= *status;
        if(!test_status)
        {
            failed_thr_i = thr_i;
            failed_group_i = 1;
        }
        free(status);

        pthread_join(threads2[thr_i],(void**)&status);
        test_status &= *status;
        if(!test_status)
        {
            failed_thr_i = thr_i;
            failed_group_i = 2;
        }
        free(status);
    }
    LP_TEST_ASSERT(test_status,"Failed to perform operation in thread %ld of %d group. Concurrent find/remove.",failed_thr_i,failed_group_i);

    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        args1[thr_i].ht = ht;
        args1[thr_i].type = __HT_OP_INSERT;
        args1[thr_i].value = thr_i+threads_num;
        pthread_create(&threads1[thr_i],NULL,__ht_op,&args1[thr_i]);

        args2[thr_i].ht = ht;
        args2[thr_i].type = __HT_OP_INSERT;
        args2[thr_i].value = thr_i+threads_num;
        pthread_create(&threads2[thr_i],NULL,__ht_op,&args2[thr_i]);
    }
    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        bool *status;
        pthread_join(threads1[thr_i],(void**)&status);
        free(status);

        pthread_join(threads2[thr_i],(void**)&status);
        free(status);
    }
    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        args1[thr_i].ht = ht;
        args1[thr_i].type = __HT_OP_FIND;
        args1[thr_i].value = thr_i+threads_num;
        pthread_create(&threads1[thr_i],NULL,__ht_op,&args1[thr_i]);
    }
    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        bool *status;
        pthread_join(threads1[thr_i],(void**)&status);
        test_status &= *status;
        if(!test_status)
            failed_thr_i = thr_i;
        free(status);
    }
    LP_TEST_ASSERT(test_status,"Entry not found in thread %ld of group 1. Concurrent insert/insert same buckets",failed_thr_i);

    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        args1[thr_i].ht = ht;
        args1[thr_i].type = __HT_OP_REMOVE;
        args1[thr_i].value = thr_i;
        pthread_create(&threads1[thr_i],NULL,__ht_op,&args1[thr_i]);

        args2[thr_i].ht = ht;
        args2[thr_i].type = __HT_OP_REMOVE;
        args2[thr_i].value = thr_i+threads_num;
        pthread_create(&threads2[thr_i],NULL,__ht_op,&args2[thr_i]);
    }
    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        bool *status;
        pthread_join(threads1[thr_i],(void**)&status);
        test_status &= *status;
        if(!test_status)
        {
            failed_thr_i = thr_i;
            failed_group_i = 1;
        }
        free(status);

        pthread_join(threads2[thr_i],(void**)&status);
        test_status &= *status;
        if(!test_status)
        {
            failed_thr_i = thr_i;
            failed_group_i = 2;
        }
        free(status);
    }
    LP_TEST_ASSERT(test_status,"Failed to perform operation in thread %ld of %d group. Concurrent remove/remove different buckets.",failed_thr_i,failed_group_i);
    
    LP_TEST_ASSERT(lp_shtable_size(ht) == 0,"At the end expected size 0, got: %ld",ht->__size);

    lp_test_cleanup:
    lp_shtable_release(ht);
    free(threads1);
    free(args1);
    free(threads2);
    free(args2);
}


void test_shtable_random_ops(size_t tests_num, size_t threads_num)
{
    for(size_t test_i = 0; test_i < tests_num; ++test_i)
    {
        LP_TEST_STEP_INTO(__test_shtable_random_ops(threads_num));
    }
    lp_test_cleanup:
}


void lp_test_shtable()
{
    LP_TEST_RUN(test_shtable_random_ops(10,1));
    LP_TEST_RUN(test_shtable_random_ops(10,4));
    LP_TEST_RUN(test_shtable_random_ops(10,20));
    LP_TEST_RUN(test_shtable_random_ops(2,1000));
}
