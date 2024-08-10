#include <lockpick/graph/inference/ocl/infer.h>
#include <lockpick/ocl/ocl.h>
#include <lockpick/affirmf.h>


void lpg_inference_graph_infer_random_inputs_ocl(lpg_inference_graph_t *inference_graph)
{
    affirm_nullptr(inference_graph,"inference graph");

    cl_context context = lp_ocl_create_context();
    
}
