#include <lockpick/ocl/print_info.h>
#include <lockpick/ocl/ocl.h>
#include <lockpick/affirmf.h>
#include <lockpick/uint.h>
#include <lockpick/math.h>
#include <lockpick/string.h>
#include <stdio.h>
#include <string.h>

#define __LP_OCL_MAX_STR_INFO_SIZE 512


lp_htable_t *__lp_ocl_device_properties_cli_fetch;

typedef struct __lp_ocl_device_properties_cli_fetch_entry
{
    char *prop_str;
    cl_device_info prop_id;
} __lp_ocl_device_properties_cli_fetch_entry_t;

size_t __lp_ocl_device_properties_cli_fetch_hsh(const __lp_ocl_device_properties_cli_fetch_entry_t *x)
{
    return lp_string_hash(x->prop_str);
}

bool __lp_ocl_device_properties_cli_fetch_eq(const __lp_ocl_device_properties_cli_fetch_entry_t *a, const __lp_ocl_device_properties_cli_fetch_entry_t *b)
{
    return !strcmp(a->prop_str,b->prop_str);
}


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


cl_device_info __lp_ocl_device_info_from_name(char *name)
{
    __lp_ocl_device_properties_cli_fetch_entry_t entry;
    entry.prop_str = name;
    affirmf(lp_htable_find(__lp_ocl_device_properties_cli_fetch,&entry,&entry),
        "Device property with name '%s' not found",name);
    
    return entry.prop_id;
}


void __lp_ocl_get_padding(const char *property_name, char *pad_str)
{
    int64_t property_name_len = (int64_t)strlen(property_name);
    size_t pad_size = __LP_OCL_PRINT_INFO_PADDING_SIZE-property_name_len;
    memset(pad_str,' ',pad_size);
    pad_str[pad_size] = '\0';
}


void __lp_ocl_get_device_type_str(cl_device_info device_type, char *dest)
{
    char *device_type_str;
    switch(device_type)
    {
        case CL_DEVICE_TYPE_CPU:
            device_type_str = "CPU";
            break;
        
        case CL_DEVICE_TYPE_GPU:
            device_type_str = "GPU";
            break;

        case CL_DEVICE_TYPE_ACCELERATOR:
            device_type_str = "Accelerator";
            break;
        
        case CL_DEVICE_TYPE_CUSTOM:
            device_type_str = "Custom";
            break;
        
        default:
            device_type_str = "Unknown";
            break;
    }

    strcpy(dest,device_type_str);
}


void __lp_ocl_print_info_platform_cb(cl_platform_id platform, cl_uint plat_i, void *args)
{
    char platform_name[__LP_OCL_MAX_STR_INFO_SIZE];
    affirmf(clGetPlatformInfo(platform,CL_PLATFORM_NAME,__LP_OCL_MAX_STR_INFO_SIZE,platform_name,NULL) == CL_SUCCESS,
        "Failed to fetch platform name string");
    
    char platform_version[__LP_OCL_MAX_STR_INFO_SIZE];
    affirmf(clGetPlatformInfo(platform,CL_PLATFORM_VERSION,__LP_OCL_MAX_STR_INFO_SIZE,platform_version,NULL) == CL_SUCCESS,
        "Failed to fetch platform version string");
    
    char platform_vendor[__LP_OCL_MAX_STR_INFO_SIZE];
    affirmf(clGetPlatformInfo(platform,CL_PLATFORM_VENDOR,__LP_OCL_MAX_STR_INFO_SIZE,platform_vendor,NULL) == CL_SUCCESS,
        "Failed to fetch platform vendor string");
    
    if(plat_i != 0)
        printf("\n");
    printf(lp_string_styled_lit("%s, %s ( %s ):\n",common,bold),platform_name,platform_vendor,platform_version);
}

void __lp_ocl_print_info_device_cb(cl_device_id device, cl_uint dev_i, void *args)
{
    printf(lp_string_styled_lit("#%d:\n",white,bold),(uint32_t)dev_i);
    char **device_params = args;
    char pad_str[__LP_OCL_PRINT_INFO_PADDING_SIZE];
    for(size_t param_i = 0; device_params[param_i] != NULL; ++param_i)
    {
        cl_device_info property = __lp_ocl_device_info_from_name(device_params[param_i]);
        size_t property_size;
        affirmf(clGetDeviceInfo(device,property,0,NULL,&property_size) == CL_SUCCESS,
            "Failed to fetch size for device property '%s'",device_params[param_i]);
        
        void *property_value = malloc(property_size);
        affirm_bad_malloc(property_value,"property value",property_size);
        affirmf(clGetDeviceInfo(device,property,property_size,property_value,NULL) == CL_SUCCESS,
            "Failed to fetch '%s' property value",device_params[param_i]);
        
        __lp_ocl_get_padding(device_params[param_i],pad_str);
        
        printf("\t%s:%s",device_params[param_i],pad_str);
        switch(property)
        {
            case CL_DEVICE_NAME:
            case CL_DEVICE_VENDOR:
            case CL_DEVICE_PROFILE:
            case CL_DEVICE_VERSION:
                printf("%s\n",(char*)property_value);
                break;

            case CL_DEVICE_MAX_COMPUTE_UNITS:
            case CL_DEVICE_MAX_CLOCK_FREQUENCY:
            case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
                printf("%d\n",(uint32_t)*(cl_uint*)property_value);
                break;
            
            case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
            case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
            case CL_DEVICE_GLOBAL_MEM_SIZE:
            case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
            case CL_DEVICE_LOCAL_MEM_SIZE:
                printf("%ld\n",(uint64_t)*(cl_ulong*)property_value);
                break;
            
            case CL_DEVICE_MAX_WORK_GROUP_SIZE:
            case CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE:
                printf("%zd\n",*(size_t*)property_value);
                break;
            
            case CL_DEVICE_AVAILABLE:
                if(*(cl_bool*)property_value)
                    printf(lp_string_styled_lit("true\n",green,bold));
                else
                    printf(lp_string_styled_lit("false\n",red,bold));
                break;
            
            case CL_DEVICE_MAX_WORK_ITEM_SIZES:
            {
                size_t *max_work_item_sizes = property_value;
                printf("[%zd,%zd,%zd]\n",max_work_item_sizes[0],max_work_item_sizes[1],max_work_item_sizes[2]);
                break;
            }

            case CL_DEVICE_UUID_KHR:
            {
                size_t uuid_hex_str_size = CL_UUID_SIZE_KHR*(LP_BITS_PER_BYTE/LP_BITS_PER_HEX)+1;
                char uuid_hex_str[uuid_hex_str_size];
                __lp_ocl_device_uuid_to_hex_str(property_value,uuid_hex_str,uuid_hex_str_size);
                printf(lp_string_styled_lit("%s\n",cyan,bold),uuid_hex_str);
                break;
            }

            case CL_DEVICE_TYPE:
            {
                cl_device_info device_type = *(cl_device_type*)property_value;
                const uint16_t device_type_str_size = 32;
                char device_type_str[device_type_str_size];
                __lp_ocl_get_device_type_str(device_type,device_type_str);
                printf("%s\n",device_type_str);
                break;
            }
            
            default:
                errorf("Unsupported property to print: '%s'",device_params[param_i]);
        }

        free(property_value);
    }
}

void lp_ocl_print_info(char **properties)
{
    affirmf(__lp_ocl_device_properties_cli_fetch != NULL,"Call '__lp_ocl_print_info_init' before to use this");
    affirm_nullptr(properties,"properties");

    lp_ocl_traverse_devices(__lp_ocl_print_info_platform_cb,NULL,__lp_ocl_print_info_device_cb,properties);
}


void __lp_ocl_print_info_init()
{
    __lp_ocl_device_properties_cli_fetch = lp_htable_create(
                    1,
                    sizeof(__lp_ocl_device_properties_cli_fetch_entry_t),
                    lp_htable_cast_hsh(__lp_ocl_device_properties_cli_fetch_hsh),
                    lp_htable_cast_eq(__lp_ocl_device_properties_cli_fetch_eq));
    
    __lp_ocl_device_properties_cli_fetch_entry_t entry;

    entry.prop_str = "CL_DEVICE_NAME";
    entry.prop_id = CL_DEVICE_NAME;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_VENDOR";
    entry.prop_id = CL_DEVICE_VENDOR;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_PROFILE";
    entry.prop_id = CL_DEVICE_PROFILE;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_VERSION";
    entry.prop_id = CL_DEVICE_VERSION;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_MAX_COMPUTE_UNITS";
    entry.prop_id = CL_DEVICE_MAX_COMPUTE_UNITS;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS";
    entry.prop_id = CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_MAX_WORK_ITEM_SIZES";
    entry.prop_id = CL_DEVICE_MAX_WORK_ITEM_SIZES;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_MAX_WORK_GROUP_SIZE";
    entry.prop_id = CL_DEVICE_MAX_WORK_GROUP_SIZE;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_MAX_CLOCK_FREQUENCY";
    entry.prop_id = CL_DEVICE_MAX_CLOCK_FREQUENCY;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_MAX_MEM_ALLOC_SIZE";
    entry.prop_id = CL_DEVICE_MAX_MEM_ALLOC_SIZE;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE";
    entry.prop_id = CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_GLOBAL_MEM_CACHE_SIZE";
    entry.prop_id = CL_DEVICE_GLOBAL_MEM_CACHE_SIZE;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_GLOBAL_MEM_SIZE";
    entry.prop_id = CL_DEVICE_GLOBAL_MEM_SIZE;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE";
    entry.prop_id = CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE";
    entry.prop_id = CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_LOCAL_MEM_SIZE";
    entry.prop_id = CL_DEVICE_LOCAL_MEM_SIZE;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_AVAILABLE";
    entry.prop_id = CL_DEVICE_AVAILABLE;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_UUID_KHR";
    entry.prop_id = CL_DEVICE_UUID_KHR;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);

    entry.prop_str = "CL_DEVICE_TYPE";
    entry.prop_id = CL_DEVICE_TYPE;
    lp_htable_insert(__lp_ocl_device_properties_cli_fetch,&entry);
}