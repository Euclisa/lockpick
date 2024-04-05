#ifndef _LOCKPICK_GRAPH_INFERENCE_HOST_INFER_H
#define _LOCKPICK_GRAPH_INFERENCE_HOST_INFER_H

#include <lockpick/graph/inference/inference_graph.h>
#include <lockpick/bitset.h>


lp_bitset_t *lpg_inference_graph_infer_host(lpg_inference_graph_t *inference_graph, const lp_bitset_t *input_values);

#endif // _LOCKPICK_GRAPH_INFERENCE_HOST_INFER_H