#ifndef _LOCKPICK_OCL_OCL_H
#define _LOCKPICK_OCL_OCL_H

#include <CL/cl.h>


void lp_ocl_traverse_devices(void (*platform_cb)(cl_platform_id, cl_uint, void *), void *platform_cb_args, void (*device_cb)(cl_device_id, cl_uint, void *), void *device_cb_args);

#endif