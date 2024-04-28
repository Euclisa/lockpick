#ifndef _LOCKPICK_OCL_OCL_H
#define _LOCKPICK_OCL_OCL_H

#include <CL/cl.h>
#include <CL/cl_ext.h>

#define LP_OCL_DEVICE_UUID_ENV_NAME "LP_OCL_DEVICE_UUID"

typedef void (*device_cb_t)(cl_platform_id, cl_device_id, cl_uint, void *);
typedef void (*platform_cb_t)(cl_platform_id, cl_uint, void *);
void lp_ocl_traverse_devices(platform_cb_t platform_cb, void *platform_cb_args, device_cb_t device_cb, void *device_cb_args);

void __lp_ocl_device_uuid_to_hex_str(const cl_uchar *uuid, char *hex_str, size_t hex_str_size);
void __lp_ocl_device_hex_str_to_device_uuid(const char *hex_str, cl_uchar *uuid);

cl_context lp_ocl_create_context();

#endif // _LOCKPICK_OCL_OCL_H