#include "rb_tree/rb_tree.h"
#include "set/set.h"
#include "sync/test.h"
#include "uint/uint.h"
#include "list/list.h"
#include "dlist/dlist.h"
#include "math/math.h"
#include "slab/slab.h"
#include "bits/bits.h"
#include "bitset/bitset.h"
#include "htable/htable.h"
#include "ndarray/ndarray.h"
#include "vector/vector.h"
#include "graph/types/uint/uint.h"
#include "graph/graph/tsort/tsort.h"
#include "graph/graph/properties/count/count.h"
#include "graph/inference/host/infer/infer.h"
#include <lockpick/test.h>
#include <lockpick/lockpick.h>
#include <lockpick/logger.h>
#include <lockpick/string.h>


int main()
{
    lp_init(LP_LOGGER_LEVEL_OFF);
    LP_TEST_BEGIN("Lockpick");
    //LP_TEST_RUN(lp_test_rb_tree(),1);
    LP_TEST_RUN(lp_test_set());
    //LP_TEST_RUN(lp_test_uint(),1);
    //LP_TEST_RUN(lp_test_dlist(),1);
    //LP_TEST_RUN(lp_test_list(),1);
    //LP_TEST_RUN(lp_test_ndarray(),1);
    //LP_TEST_RUN(lp_test_math(),1);
    //LP_TEST_RUN(lp_test_slab(),1);
    //LP_TEST_RUN(lp_test_bits(),1);
    //LP_TEST_RUN(lp_test_bitset(),1);
    //LP_TEST_RUN(lp_test_vector(),1);
    //LP_TEST_RUN(lp_test_sync());
    //LP_TEST_RUN(lp_test_htable(),1);
    //LP_TEST_RUN(lp_test_graph_uint(),1);
    //LP_TEST_RUN(lp_test_graph_tsort(),1);
    //LP_TEST_RUN(lp_test_graph_count(),1);
    //LP_TEST_RUN(lp_test_inference_graph_infer_host(),1);
    LP_TEST_END();
}