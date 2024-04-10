#include <lockpick/string.h>
#include <lockpick/utility.h>
#include <lockpick/affirmf.h>


size_t lp_string_hash(const char *str)
{
    affirm_nullptr(str,"string");

    size_t seed = 1;
    size_t curr_char_i = 0;
    while(str[curr_char_i])
    {
        size_t char_hash = lp_uni_hash(str[curr_char_i]);
        seed ^= lp_uni_hash(curr_char_i+char_hash);
        ++curr_char_i;
    }

    return seed;
}