#include <lockpick/graph/functions/sha256.h>


lpg_graph_t *lpg_sha256_create_graph(size_t input_size)
{
    size_t padded_input_size = input_size + input_size % LPG_SHA256_PADDED_INPUT_CHUNCK_SIZE;
    size_t chuncks_number = padded_input_size / LPG_SHA256_PADDED_INPUT_CHUNCK_SIZE;
    size_t total_nodes_upper_bound = LPG_SHA256_UPPER_BOUND_NODES_PER_ITER * chuncks_number;

    lpg_graph_t *graph = lpg_graph_create("SHA-256",input_size,LPG_SHA256_OUTPUT_SIZE,total_nodes_upper_bound);
}