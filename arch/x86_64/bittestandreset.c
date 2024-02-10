#include <lockpick/arch/x86_64/bittestandreset.h>


uint8_t lp_bittestandreset(uint32_t *bitmap, uint32_t bit_offset)
{
    uint8_t result;
    __asm__ volatile (
            "lock btr %[off],%[bm]\n\t"
          : [res]"=@ccc"(result), [bm]"+m"(*bitmap)
          : [off]"r"(bit_offset)
          : "memory");
    
    return result;
}