#include <lockpick/spinlock_bitset.h>
#include <lockpick/define.h>
#include <lockpick/affirmf.h>
#include <lockpick/math.h>
#include <lockpick/arch/x86_64/bittestandset.h>
#include <lockpick/arch/x86_64/bittestandreset.h>
#include <stdlib.h>
#include <immintrin.h>


lp_spinlock_bitset_t *lp_spinlock_bitset_init(size_t locks_num)
{
    size_t spins_size = sizeof(lp_spinlock_bitset_t);
    lp_spinlock_bitset_t *spins = (lp_spinlock_bitset_t*)malloc(spins_size);
    affirm_bad_malloc(spins,"spinlocks bitset object",spins_size);

    size_t bitset_words_num = lp_ceil_div_u(locks_num,(sizeof(uint32_t)*LP_BITS_IN_BYTE));
    size_t bitset_size = bitset_words_num*sizeof(uint32_t);
    spins->__bitset = (uint32_t*)calloc(bitset_words_num,sizeof(uint32_t));
    affirm_bad_malloc(spins->__bitset,"spinlocks bitset",bitset_size);

    spins->__locks_num = locks_num;

    return spins;
}


void lp_spinlock_bitset_release(lp_spinlock_bitset_t *spins)
{
    affirm_nullptr(spins,"spinlocks bitset structure");

    free(spins->__bitset);
    free(spins);
}


bool lp_spinlock_bitset_lock(lp_spinlock_bitset_t *spins, size_t lock_i)
{
    if(__unlikely(!spins || lock_i >= spins->__locks_num))
        return_set_errno(false,EINVAL);

    size_t lock_word_i = lock_i / (sizeof(uint32_t)*LP_BITS_IN_BYTE);
    uint32_t lock_bit_i = (uint32_t)(lock_i % (sizeof(uint32_t)*LP_BITS_IN_BYTE));

    uint32_t *lock_word = spins->__bitset+lock_word_i;
    while(lp_bittestandset(lock_word,lock_bit_i))
    {
        while(*lock_word & ((uint32_t)1 << lock_bit_i))
            _mm_pause();
    }

    return true;
}


bool lp_spinlock_bitset_unlock(lp_spinlock_bitset_t *spins, size_t lock_i)
{
    if(__unlikely(!spins || lock_i >= spins->__locks_num))
        return_set_errno(false,EINVAL);
    
    size_t lock_word_i = lock_i / (sizeof(uint32_t)*LP_BITS_IN_BYTE);
    uint32_t lock_bit_i = (uint32_t)(lock_i % (sizeof(uint32_t)*LP_BITS_IN_BYTE));

    uint32_t *lock_word = spins->__bitset+lock_word_i;
    if(!lp_bittestandreset(lock_word,lock_bit_i))
        return_set_errno(false,ENLCKD);
    
    return true;
}


bool lp_spinlock_bitset_trylock(lp_spinlock_bitset_t *spins, size_t lock_i)
{
    if(__unlikely(lock_i >= spins->__locks_num))
        return_set_errno(false,EINVAL);

    size_t lock_word_i = lock_i / (sizeof(uint32_t)*LP_BITS_IN_BYTE);
    uint32_t lock_bit_i = (uint32_t)(lock_i % (sizeof(uint32_t)*LP_BITS_IN_BYTE));

    uint32_t *lock_word = spins->__bitset+lock_word_i;
    if(lp_bittestandset(lock_word,lock_bit_i))
        return_set_errno(false,EBUSY);

    return true;
}