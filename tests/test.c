#include "rb_tree/rb_tree.h"
#include "lp_uint/lp_uint.h"
#include "lp_test.h"

int main()
{
    LP_TEST_BEGIN("Lockpick");
    LP_TEST_RUN(test_rb_tree());
    LP_TEST_RUN(test_lp_uint());
    LP_TEST_END();
}