#include "lock_graph/lock_graph.h"
#include "spinlock_bitset/spinlock_bitset.h"
#include "shtable/shtable.h"
#include <lockpick/test.h>


void lp_test_sync()
{
    //LP_TEST_RUN(lp_test_lock_graph(),1);
    //LP_TEST_RUN(lp_test_spinlock_bitset(),1);
    LP_TEST_RUN(lp_test_shtable());
}