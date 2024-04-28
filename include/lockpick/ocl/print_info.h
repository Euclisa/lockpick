#ifndef _LOCKPICK_OCL_PRINT_INFO_H
#define _LOCKPICK_OCL_PRINT_INFO_H

#include <lockpick/htable.h>
#include <CL/cl.h>
#include <CL/cl_ext.h>


typedef struct lp_ocl_device_props
{
    char *name;
    char *vendor;
    char *profile;
    char *version;
    cl_uint max_compute_units;
    cl_uint max_work_item_dimensions;
    size_t max_work_item_sizes;
    size_t max_work_group_size;
    cl_uint max_clock_frequency;
    cl_ulong max_mem_alloc_size;
    cl_uint global_mem_cacheline_size;
    cl_ulong global_mem_cache_size;
    cl_ulong global_mem_size;
    cl_ulong max_constant_buffer_size;
    size_t max_global_variable_size;
    cl_ulong local_mem_size;
    cl_bool available;
} lp_ocl_device_props_t;

#define __LP_OCL_PRINT_INFO_PADDING_SIZE 48


extern lp_htable_t *__lp_ocl_device_properties_cli_fetch;


void lp_ocl_print_info(char **properties);

void __lp_ocl_print_info_init();

#endif // _LOCKPICK_OCL_PRINT_INFO_H