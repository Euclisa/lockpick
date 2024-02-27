#include "rb_tree/rb_tree.h"
#include "sync/test.h"
#include "uint/uint.h"
#include "list/list.h"
#include "math/math.h"
#include "slab/slab.h"
#include "bits/bits.h"
#include "htable/htable.h"
#include "graph/types/uint/uint.h"
#include <lockpick/test.h>

int main()
{
    LP_TEST_BEGIN("Lockpick");
    //LP_TEST_RUN(lp_test_rb_tree(),1);
    //LP_TEST_RUN(lp_test_uint(),1);
    //LP_TEST_RUN(lp_test_list(),1);
    //LP_TEST_RUN(lp_test_math());
    //LP_TEST_RUN(lp_test_slab(),1);
    //LP_TEST_RUN(lp_test_bits(),1);
    //LP_TEST_RUN(lp_test_sync());
    //LP_TEST_RUN(lp_test_htable(),1);
    LP_TEST_RUN(lp_test_graph_uint());
    LP_TEST_END();
}