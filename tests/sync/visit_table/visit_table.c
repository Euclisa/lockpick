#include <lockpick/test.h>
#include <lockpick/sync/visit_table.h>
#include <lockpick/affirmf.h>
#include <lockpick/utility.h>
#include <lockpick/errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>


static size_t __u32_hsh(const void *x)
{
    size_t ux = *(uint32_t*)x;
    return lp_uni_hash(ux);
}


static bool __u32_eq(const void *a, const void *b)
{
    uint32_t ua = *(uint32_t*)a;
    uint32_t ub = *(uint32_t*)b;
    return ua == ub;
}


enum __vt_op_type
{
    __VT_OP_INSERT,
    __VT_OP_FIND
};


typedef struct __vt_op_arg
{
    lp_visit_table_t *vt;
    enum __vt_op_type type;
    uint32_t value;
} __vt_op_arg_t;


void *__vt_op(void *_arg)
{
    __vt_op_arg_t *arg = (__vt_op_arg_t*)_arg;
    lp_visit_table_t *vt = arg->vt;
    enum __vt_op_type type = arg->type;
    uint32_t value = arg->value;
    bool *status = (bool*)malloc(sizeof(bool));
    
    if(type == __VT_OP_INSERT)
        *status = lp_visit_table_insert(vt,&value);
    else if(type == __VT_OP_FIND)
    {
        uint32_t found;
        *status = lp_visit_table_find(vt,&value,&found);
        if(*status)
            *status = found == value;
    }

    return status;
}


void __test_visit_table_random_ops(size_t threads_num)
{
    lp_visit_table_t *vt = lp_visit_table_create(threads_num*2,sizeof(uint32_t),__u32_hsh,__u32_eq);
    
    pthread_t *threads1 = (pthread_t*)malloc(threads_num*sizeof(pthread_t));
    __vt_op_arg_t *args1 = (__vt_op_arg_t*)malloc(threads_num*sizeof(__vt_op_arg_t));
    pthread_t *threads2 = (pthread_t*)malloc(threads_num*sizeof(pthread_t));
    __vt_op_arg_t *args2 = (__vt_op_arg_t*)malloc(threads_num*sizeof(__vt_op_arg_t));
    
    size_t failed_thr_i = 0;
    uint32_t failed_group_i = 0;
    bool test_status = true;
    bool inserted = true;
    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        args1[thr_i].vt = vt;
        args1[thr_i].type = __VT_OP_INSERT;
        args1[thr_i].value = thr_i;
        pthread_create(&threads1[thr_i],NULL,__vt_op,&args1[thr_i]);

        args2[thr_i].vt = vt;
        args2[thr_i].type = __VT_OP_INSERT;
        args2[thr_i].value = thr_i;
        pthread_create(&threads2[thr_i],NULL,__vt_op,&args2[thr_i]);
    }
    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        bool *status1;
        pthread_join(threads1[thr_i],(void**)&status1);
        bool *status2;
        pthread_join(threads2[thr_i],(void**)&status2);
        bool xor_status = *status1 ^ *status2;
        if(test_status && !xor_status)
        {
            test_status = false;
            failed_thr_i = thr_i;
            inserted = *status1;
        }

        free(status1);
        free(status2);
    }
    LP_TEST_ASSERT(test_status,
        "In thread %ld spotted equal insertion status: %d",failed_thr_i,(uint32_t)inserted);

    test_status = true;
    for(size_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        args1[thr_i].vt = vt;
        args1[thr_i].type = __VT_OP_INSERT;
        args1[thr_i].value = thr_i+threads_num;
        pthread_create(&threads1[thr_i],NULL,__vt_op,&args1[thr_i]);

        args2[thr_i].vt = vt;
        args2[thr_i].type = __VT_OP_FIND;
        args2[thr_i].value = thr_i;
        pthread_create(&threads2[thr_i],NULL,__vt_op,&args2[thr_i]);
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
    LP_TEST_ASSERT(test_status,
        "Failed to perform operation in thread %ld of %d group. Concurrent find/insert.",
        failed_thr_i,failed_group_i);

    uint32_t entry = (uint32_t)-1;
    bool nocap_insertion_status = lp_visit_table_insert(vt,&entry);
    LP_TEST_ASSERT(!nocap_insertion_status && errno == ENCAP,
        "Expected error status with ENCAP errno");

    lp_test_cleanup:
    lp_visit_table_release(vt);
    free(threads1);
    free(args1);
    free(threads2);
    free(args2);
}


void test_visit_table_random_ops(size_t tests_num, size_t threads_num)
{
    for(size_t test_i = 0; test_i < tests_num; ++test_i)
    {
        LP_TEST_STEP_INTO(__test_visit_table_random_ops(threads_num));
    }
    lp_test_cleanup:
}


void lp_test_visit_table()
{
    LP_TEST_RUN(test_visit_table_random_ops(1000,4));
    LP_TEST_RUN(test_visit_table_random_ops(1000,8));
    LP_TEST_RUN(test_visit_table_random_ops(100,16));
    LP_TEST_RUN(test_visit_table_random_ops(50,128));
    LP_TEST_RUN(test_visit_table_random_ops(20,512));
}