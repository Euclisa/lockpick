#include <lockpick/ocl/print_info.h>


int main(int argc, char *argv[])
{
    __lp_ocl_print_info_init();

    char **properties = argv+1;
    lp_ocl_print_info(properties);

    return 0;
}