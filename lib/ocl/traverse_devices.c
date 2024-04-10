#include <lockpick/ocl/ocl.h>
#include <lockpick/affirmf.h>
#include <stdio.h>
#include <malloc.h>


void lp_ocl_traverse_devices(void (*platform_cb)(cl_platform_id, cl_uint, void *), void *platform_cb_args, void (*device_cb)(cl_device_id, cl_uint, void *), void *device_cb_args)
{
    cl_uint status;

    cl_uint num_platforms;
    affirmf(clGetPlatformIDs(0,NULL,&num_platforms) == CL_SUCCESS,"Failed to fetch platforms number");

    affirmf(num_platforms > 0,"Failed to found any OpenCL platforms");

    size_t platforms_size = num_platforms*sizeof(cl_platform_id);
    cl_platform_id *platforms = (cl_platform_id*)malloc(platforms_size);
    affirm_bad_malloc(platforms,"platforms ids array",platforms_size);

    affirmf(clGetPlatformIDs(num_platforms,platforms,NULL) == CL_SUCCESS,"Failed to fetch available platforms");

    for(cl_uint plat_i = 0; plat_i < num_platforms; ++plat_i)
    {
        if(platform_cb)
            platform_cb(platforms[plat_i],plat_i,platform_cb_args);

        cl_uint num_devices;
        status = clGetDeviceIDs(platforms[plat_i],CL_DEVICE_TYPE_ALL,0,NULL,&num_devices);
        if(status == CL_DEVICE_NOT_FOUND) continue;
        affirmf(clGetDeviceIDs(platforms[plat_i],CL_DEVICE_TYPE_ALL,0,NULL,&num_devices) == CL_SUCCESS,
            "Failed to fetch devices number for plaform %d",(uint32_t)plat_i);
        
        size_t devices_size = num_devices*sizeof(cl_device_id);
        cl_device_id *devices = (cl_device_id*)malloc(devices_size);
        affirm_bad_malloc(devices,"devices ids array",devices_size);

        affirmf(clGetDeviceIDs(platforms[plat_i],CL_DEVICE_TYPE_ALL,num_devices,devices,NULL) == CL_SUCCESS,
            "Failed to fetch devices for platform %d",(uint32_t)plat_i);
        
        for(cl_uint dev_i = 0; dev_i < num_devices; ++dev_i)
        {
            if(device_cb)
                device_cb(devices[dev_i],dev_i,device_cb_args);
        }

        free(devices);
    }

    free(platforms);
}