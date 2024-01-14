#include "rb_tree/rb_tree.h"
#include "uint/uint.h"
#include "test.h"

int main()
{
    LP_TEST_BEGIN("Lockpick");
    LP_TEST_RUN(test_rb_tree(),1);
    LP_TEST_RUN(test_lp_uint(),1);
    LP_TEST_END();
}