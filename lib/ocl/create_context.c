#include <lockpick/ocl/ocl.h>
#include <lockpick/affirmf.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


typedef struct __lp_ocl_find_device_from_uuid_entry
{
    cl_device_id device;
    cl_uchar *uuid;
    bool found;
} __lp_ocl_find_device_from_uuid_entry_t;

void __lp_ocl_find_device_from_uuid(cl_platform_id platform, cl_device_id device, cl_uint index, __lp_ocl_find_device_from_uuid_entry_t *args)
{
    cl_uchar curr_device_uuid[CL_UUID_SIZE_KHR];
    affirmf(clGetDeviceInfo(device,CL_DEVICE_UUID_KHR,CL_UUID_SIZE_KHR,curr_device_uuid,NULL) == CL_SUCCESS,
        "Failed to fetch device uuid during traveral");

    if(!memcmp(args->uuid,curr_device_uuid,CL_UUID_SIZE_KHR))
    {
        affirmf(!args->found,"Found two devices with identical uuid");
        args->device = device;
        args->found = true;
    }
}


cl_context lp_ocl_create_context()
{
    const char *device_uuid_hex = getenv(LP_OCL_DEVICE_UUID_ENV_NAME);
    affirmf(device_uuid_hex != NULL,
        "Set '%s' environment variable to desirable OpenCL device uuid",LP_OCL_DEVICE_UUID_ENV_NAME);
    
    cl_uchar device_uuid[CL_UUID_SIZE_KHR];
    __lp_ocl_device_hex_str_to_device_uuid(device_uuid_hex,device_uuid);
    
    __lp_ocl_find_device_from_uuid_entry_t args;
    args.uuid = device_uuid;
    args.found = false;
    lp_ocl_traverse_devices(
        NULL,NULL,
        (device_cb_t)__lp_ocl_find_device_from_uuid,&args);
    affirmf(args.found,"Device with uuid '%s' not found",device_uuid_hex);

    cl_device_id device = args.device;
    cl_int errcode;
    cl_context context = clCreateContext(NULL,1,&device,NULL,NULL,&errcode);
    affirmf(errcode == CL_SUCCESS,"Failed to create context for device with uuid '%s'",device_uuid_hex);

    return context;
}
