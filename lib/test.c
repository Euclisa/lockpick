#include <lockpick/test.h>
#include <lockpick/affirmf.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __LP_TEST_PASSED_STYLE_MAGIC "\033[1;32;1m"
#define __LP_TEST_RUNNING_STYLE_MAGIC "\033[1;33;1m"
#define __LP_TEST_FAILED_STYLE_MAGIC "\033[1;31;1m"
#define __LP_TEST_EMPTY_STYLE_MAGIC "\033[1;34;1m"
#define __LP_TEST_RESET_STYLE_MAGIC "\033[0m"

#define __LP_TEST_NANO_DECIMALS 1000000000
#define __LP_TEST_MICRO_DECIMALS 1000000
#define __LP_TEST_MILLI_DECIMALS 1000

#define __LP_TEST_ALLOCATOR_CLEAR ((size_t)-1)
#define __LP_TEST_ALLOCATOR_BUFF_SIZE 1024

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


char *__lp_test_allocator(size_t size)
{
    static char buffer[__LP_TEST_ALLOCATOR_BUFF_SIZE];
    static size_t offset = 0;
    
    if(size == __LP_TEST_ALLOCATOR_CLEAR)
    {
        offset = 0;
        return buffer;
    }

    affirmf((__LP_TEST_ALLOCATOR_BUFF_SIZE-offset) >= size, "Failed to allocate %ld bytes. Out of memory.",size);

    char *to_ret = buffer+offset;
    offset += size;

    return to_ret;
}


char *__get_time_str(const char *format)
{
    const uint8_t MAX_TIME_STR_SIZE = 64; 

    time_t now = time(NULL);

    char *time_str = __lp_test_allocator(MAX_TIME_STR_SIZE+1);
    affirmf(time_str,"Failed to allocate string of length %ld",MAX_TIME_STR_SIZE);

    affirmf(strftime(time_str, MAX_TIME_STR_SIZE, format, localtime(&now)) > 0,"Failed to fetch local time"); 
    
    return time_str;
}


void __lp_test_print_init(const char *project_name_str)
{
    char *time_str = __get_time_str("%x %X");
    printf("*** Quality Assurance for '%s' (%s) ***\n",project_name_str,time_str);
}


char *__create_padding(uint8_t curr_level)
{
    uint16_t pad_size = (curr_level)*2;
    char *str = __lp_test_allocator(pad_size+1);
    affirmf(str,"Failed to allocate string of length %ld",pad_size);
    
    for(size_t i = 0; i < pad_size; ++i)
        str[i] = i % 2 ? ' ' : '|';
    str[pad_size] = '\0';

    return str;
}


void __lp_test_print_enter(uint8_t curr_level, const char *test_call_str)
{
    char *space_padding = __create_padding(curr_level);
    char *time_str = __get_time_str("%X");
    printf("%s|-+-> [%s] %sRUNNING%s Test: %s\n",
        space_padding,time_str,__LP_TEST_RUNNING_STYLE_MAGIC,__LP_TEST_RESET_STYLE_MAGIC,test_call_str);
}


char *__lp_test_get_stats_str(const char *test_call_str, uint64_t tests_passed, uint64_t cases_passed, uint64_t duration_total_ns)
{
    uint64_t duration_total_ms = duration_total_ns/(__LP_TEST_NANO_DECIMALS/__LP_TEST_MILLI_DECIMALS);
    uint64_t duration_per_case_ns = duration_total_ns / MAX(1,cases_passed);

    const char *stats_format = "Test: %s | TP: %ld | CP: %ld | DT: %ld ms | DPC: %ld ns";
    size_t stats_str_len = snprintf(NULL,0,stats_format,
        test_call_str,tests_passed,cases_passed,duration_total_ms,duration_per_case_ns)+1;

    char *stats_str = __lp_test_allocator(stats_str_len+1);
    affirmf(stats_str,"Failed to allocate string of length %ld",stats_str_len);

    uint64_t chars_written = snprintf(stats_str,stats_str_len,stats_format,
        test_call_str,tests_passed,cases_passed,duration_total_ms,duration_per_case_ns);
    affirmf(chars_written > 0, "Failed to write formatted string");
    
    return stats_str;
}


/**
 * __lp_test_print_leave_status - prints status message on '__LP_TEST_LEAVE' event.
 * @curr_level:                 level of current test
 * @test_call_str:              name of test call
 * @tests_failed:               number of tests failed (meaning test groups, not cases)
 * @tests_total:                total number of tests (meaning test groups, not cases)
 * @cases_passed:               total number of cases passed in all groups below
 * @duration_total_ns:          total duration between current '__LP_TEST_LEAVE__LP_TEST_LEAVE' and its corresponding '__LP_TEST_ENTER' events
 * @last_failed_test_call_str:  name of last failed test call
 * @failed_test_msg:            message of last failed test case
 * 
 * Returns nothing.
*/
void __lp_test_print_leave_status(uint8_t curr_level, const char *test_call_str, uint64_t tests_failed, uint64_t tests_total, uint64_t cases_passed, uint64_t duration_total_ns, char *last_failed_test_call_str, char *failed_test_msg)
{
    bool failure = false;
    char *space_padding = __create_padding(curr_level);
    char *time_str = __get_time_str("%X");

    const size_t status_str_len = 32;
    char *status_str = __lp_test_allocator(status_str_len+1);

    uint64_t tests_passed = tests_total - tests_failed;
    bool print_stats = true;
    bool print_details = false;

    if(cases_passed == 0 && tests_failed == 0)
    {
        snprintf(status_str,status_str_len,"%sEMPTY%s",__LP_TEST_EMPTY_STYLE_MAGIC,__LP_TEST_RESET_STYLE_MAGIC);
        print_stats = false;
    }
    else if(tests_failed > 0)
    {
        snprintf(status_str,status_str_len,"%sFAILED%s",__LP_TEST_FAILED_STYLE_MAGIC,__LP_TEST_RESET_STYLE_MAGIC);
        print_details = last_failed_test_call_str[0] != '\0';
        failure = true;
    }
    else
        snprintf(status_str,status_str_len,"%sPASSED%s",__LP_TEST_PASSED_STYLE_MAGIC,__LP_TEST_RESET_STYLE_MAGIC);
    
    printf("%s|-+-> [%s] %s ",space_padding,time_str,status_str);

    if(print_stats)
    {
        char *stats_str = __lp_test_get_stats_str(test_call_str,tests_passed,cases_passed,duration_total_ns);
        printf("%s",stats_str);
    }
    if(print_details)
        printf(" | Details ('%s'): %s",last_failed_test_call_str,failed_test_msg);

    printf("\n");
}


/**
 * __lp_test_print_end - prints status message on '__LP_TEST_END' event.
 * @project_name_str:       name of project corresponding to session
 * @tests_total:            total number of tests in session
 * @duration_total_ns:      total duration of test session
 * @failure:                flag that represents status of test session
 * 
 * Returns nothing.
*/
void __lp_test_print_end(const char *project_name_str, uint64_t tests_total, uint64_t duration_total_ns, bool failure)
{
    uint64_t duration_total_ms = duration_total_ns/(__LP_TEST_NANO_DECIMALS/__LP_TEST_MILLI_DECIMALS);

    if(failure)
        printf("*** Quality Assurance for '%s' %sFAILED%s (%ld ms) ***\n",
            project_name_str,__LP_TEST_FAILED_STYLE_MAGIC,__LP_TEST_RESET_STYLE_MAGIC,duration_total_ms);
    else
        printf("*** Quality Assurance for '%s' %sPASSED%s (%ld ms) ***\n",
            project_name_str,__LP_TEST_PASSED_STYLE_MAGIC,__LP_TEST_RESET_STYLE_MAGIC,duration_total_ms);
}


/**
 * __lp_test_process_action - processes action of user program towards test session.
 * @action:       action type
 * 
 * Returns nothing.
 * 
 * This is not supposed to be called by user. One should use predefined macro definitions instead.
*/
void __lp_test_process_action(__lp_test_actions_t action, ...)
{
    static struct timespec session_init_ts;
    static char project_name_str[64] = {0};
    static char last_test_call_str[64] = {0};
    static char last_failed_test_call_str[64] = {0};
    static char failed_test_msg[1024] = {0};
    static uint8_t current_level = 0;
    static uint8_t level_max_print_depth[__LP_TEST_MAX_LEVELS] = {0};
    static uint64_t level_tests_total[__LP_TEST_MAX_LEVELS] = {0};
    static uint64_t level_cases_passed[__LP_TEST_MAX_LEVELS] = {0};
    static uint64_t level_tests_failed[__LP_TEST_MAX_LEVELS] = {0};
    static uint64_t tests_failed_before_step_in = 0;

    va_list args;
    va_start(args, action);

    switch(action)
    {
        case __LP_TEST_INIT:
        {
            affirmf(current_level == 0,"'__LP_TEST_INIT' must be invoked at zero level");

            clock_gettime(CLOCK_MONOTONIC,&session_init_ts);
    
            const char *project_name_str_obt = va_arg(args, const char*);
            affirmf(project_name_str_obt,"Project name must be not null");
            snprintf(project_name_str,sizeof(project_name_str),"%s",project_name_str_obt);

            level_max_print_depth[0] = __LP_TEST_MAX_LEVELS+1;

            __lp_test_print_init(project_name_str);
            break;
        }
        case __LP_TEST_ENTER:
        {
            affirmf(current_level < __LP_TEST_MAX_LEVELS-1,"Max number of test levels in tree exceeded (%d).",__LP_TEST_MAX_LEVELS);
            affirmf(!(level_tests_total[current_level] == 1 && level_cases_passed[current_level] > 0),"Can't branch lower level test - current test node is a leaf.");

            ++current_level;
            const char *test_call_str = va_arg(args, const char*);
            strcpy(last_test_call_str,test_call_str);
            uint64_t curr_test_max_print_depth = va_arg(args, uint64_t);
            uint8_t prev_test_max_print_depth = level_max_print_depth[current_level-1] == 0 ? 0 : level_max_print_depth[current_level-1]-1;
            level_max_print_depth[current_level] = MIN(prev_test_max_print_depth,curr_test_max_print_depth);
            if(level_max_print_depth[current_level] > 0)
                __lp_test_print_enter(current_level-1,test_call_str);
            level_tests_total[current_level] = 1;
            level_tests_failed[current_level] = 0;
            level_cases_passed[current_level] = 0;
            break;
        }
        case __LP_TEST_LEAVE:
        {
            affirmf(current_level > 0,"Can't call '__LP_TEST_LEAVE' before '__LP_TEST_ENTER'");

            const char *test_call_str = va_arg(args, const char*);
            struct timespec test_start_ts = va_arg(args,struct timespec);
            struct timespec test_end_ts = va_arg(args,struct timespec);
            uint64_t duration_total_ns = 
                (test_end_ts.tv_sec-test_start_ts.tv_sec)*__LP_TEST_NANO_DECIMALS +
                (test_end_ts.tv_nsec-test_start_ts.tv_nsec);
    
            --current_level;

            level_tests_failed[current_level] += level_tests_failed[current_level+1];
            level_tests_total[current_level] += level_tests_total[current_level+1];
            level_cases_passed[current_level] += level_cases_passed[current_level+1];

            if(level_max_print_depth[current_level+1] > 0)
            {
                __lp_test_print_leave_status(
                    current_level,
                    test_call_str,
                    level_tests_failed[current_level+1],
                    level_tests_total[current_level+1],
                    level_cases_passed[current_level+1],
                    duration_total_ns,
                    last_failed_test_call_str,
                    failed_test_msg
                );

                bool last_test_failed = level_tests_failed[current_level+1] > 0;
                if(last_test_failed)
                {
                    strcpy(failed_test_msg,"");
                    strcpy(last_failed_test_call_str,"");
                }
            }
            break;
        }
        case __LP_TEST_PASS:
        {
            affirmf(current_level > 0,"Can't call '__LP_TEST_PASS' before '__LP_TEST_ENTER'");
            affirmf(level_tests_total[current_level] == 1,"Can't assert on the current level - current test is not leaf.");

            ++level_cases_passed[current_level];
            break;
        }
        case __LP_TEST_FAIL:
        {
            affirmf(level_tests_total[current_level] == 1,"Can't assert on the current level - current test is not leaf.");

            ++level_tests_failed[current_level];
            const char *format_fail_msg = va_arg(args,const char*);
            vsnprintf(failed_test_msg,sizeof(failed_test_msg)-1,format_fail_msg,args);
            snprintf(last_failed_test_call_str,sizeof(last_failed_test_call_str)-1,"%s",last_test_call_str);
            break;
        }
        case __LP_TEST_END:
        {
            affirmf(current_level == 0,"'__LP_TEST_END' must be invoked at zero level.");

            struct timespec session_end_ts;
            clock_gettime(CLOCK_MONOTONIC,&session_end_ts);

            uint64_t duration_total_ns =
                (session_end_ts.tv_sec-session_init_ts.tv_sec)*__LP_TEST_NANO_DECIMALS +
                (session_end_ts.tv_nsec-session_init_ts.tv_nsec);

            bool session_failed = level_tests_failed[current_level] > 0;
            __lp_test_print_end(project_name_str,level_cases_passed[current_level],duration_total_ns,session_failed);
            break;
        }
        case __LP_TEST_STEP_OUT:
        {
            bool *leave_test = va_arg(args, bool*);
            *leave_test = level_tests_total[current_level] == 1 && level_tests_failed[current_level] > 0;
            break;
        }
        default:
        {
            affirmf(false,"Invalid action (%d)",(uint16_t)action);
            break;
        }
    }

    __lp_test_allocator(__LP_TEST_ALLOCATOR_CLEAR);
}