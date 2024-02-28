#include <lockpick/sync/spinlock_bitset.h>
#include <lockpick/define.h>
#include <lockpick/affirmf.h>
#include <lockpick/math.h>
#include <lockpick/sync/bits.h>
#include <lockpick/emalloc.h>
#include <stdlib.h>
#include <immintrin.h>


lp_spinlock_bitset_t *lp_spinlock_bitset_create(size_t locks_num)
{
    const size_t spins_size = sizeof(lp_spinlock_bitset_t);
    lp_spinlock_bitset_t *spins = (lp_spinlock_bitset_t*)malloc(spins_size);
    affirm_bad_malloc(spins,"spinlock bitset",spins_size);

    size_t bitset_words_num = lp_ceil_div_u64(locks_num,(sizeof(uint32_t)*LP_BITS_PER_BYTE));
    spins->__bitset = (uint32_t*)calloc(bitset_words_num,sizeof(uint32_t));
    affirm_bad_malloc(spins->__bitset,"bitset",bitset_words_num*sizeof(uint32_t));

    spins->__locks_num = locks_num;

    return spins;
}


void lp_spinlock_bitset_release(lp_spinlock_bitset_t *spins)
{
    affirm_nullptr(spins,"spinlocks bitset structure");

    free(spins->__bitset);
    free(spins);
}


void lp_spinlock_bitset_lock(lp_spinlock_bitset_t *spins, size_t lock_i)
{
    affirm_nullptr(__likely(spins),"spinlock bitset");
    affirmf(__likely(lock_i < spins->__locks_num),"Spinlock index %ld is out of range (max: %ld)",lock_i,spins->__locks_num);

    size_t lock_word_i = lock_i / (sizeof(uint32_t)*LP_BITS_PER_BYTE);
    uint32_t lock_bit_i = (uint32_t)(lock_i % (sizeof(uint32_t)*LP_BITS_PER_BYTE));

    uint32_t *lock_word = spins->__bitset+lock_word_i;
    while(lp_atomic_bittestandset(lock_word,lock_bit_i))
    {
        while(*lock_word & ((uint32_t)1 << lock_bit_i))
            _mm_pause();
    }
}


void lp_spinlock_bitset_unlock(lp_spinlock_bitset_t *spins, size_t lock_i)
{
    affirm_nullptr(__likely(spins),"spinlock bitset");
    affirmf(__likely(lock_i < spins->__locks_num),"Spinlock index %ld is out of range (max: %ld)",lock_i,spins->__locks_num);

    const size_t bits_in_spins_bm = lp_sizeof_bits(uint32_t);
    size_t lock_word_i = lp_div_pow_2(lock_i,bits_in_spins_bm);
    size_t lock_bit_i = (uint32_t)lp_mod_pow_2(lock_i,bits_in_spins_bm);

    uint32_t *lock_word = spins->__bitset+lock_word_i;
    affirmf(lp_atomic_bittestandreset(lock_word,lock_bit_i),"Spinlock %ld is not locked",lock_i);
}


bool lp_spinlock_bitset_trylock(lp_spinlock_bitset_t *spins, size_t lock_i)
{
    affirm_nullptr(__likely(spins),"spinlock bitset");
    affirmf(__likely(lock_i < spins->__locks_num),"Spinlock index %ld is out of range (max: %ld)",lock_i,spins->__locks_num);

    const size_t bits_in_spins_bm = lp_sizeof_bits(uint32_t);
    size_t lock_word_i = lp_div_pow_2(lock_i,bits_in_spins_bm);
    size_t lock_bit_i = (uint32_t)lp_mod_pow_2(lock_i,bits_in_spins_bm);

    uint32_t *lock_word = spins->__bitset+lock_word_i;
    if(lp_atomic_bittestandset(lock_word,lock_bit_i))
        return_set_errno(false,EBUSY);

    return true;
}