#include <lockpick/ocl/ocl.h>
#include <lockpick/uint.h>
#include <lockpick/affirmf.h>
#include <string.h>


void __lp_ocl_device_uuid_to_hex_str(const cl_uchar *uuid, char *hex_str, size_t hex_str_size)
{
    lp_uint256_t device_uuid_uint;
    const size_t device_uuid_uint_size = sizeof(device_uuid_uint);
    affirmf(CL_UUID_SIZE_KHR <= device_uuid_uint_size,
        "Uint type of size %zd bytes is too small for OpenCL device UUID of size %zd",device_uuid_uint_size,CL_UUID_SIZE_KHR);
    
    lp_uint_from_hex(device_uuid_uint,"0");
    memcpy(&device_uuid_uint,uuid,CL_UUID_SIZE_KHR);

    lp_uint_to_hex(device_uuid_uint,hex_str,hex_str_size);
}

void __lp_ocl_device_hex_str_to_device_uuid(const char *hex_str, cl_uchar *uuid)
{
    lp_uint256_t device_uuid_uint;
    const size_t device_uuid_uint_size = sizeof(device_uuid_uint);
    affirmf(CL_UUID_SIZE_KHR <= device_uuid_uint_size,
        "Uint type of size %zd bytes is too small for OpenCL device UUID of size %zd",device_uuid_uint_size,CL_UUID_SIZE_KHR);
    
    lp_uint_from_hex(device_uuid_uint,hex_str);
    
    memcpy(uuid,&device_uuid_uint,CL_UUID_SIZE_KHR);
}
