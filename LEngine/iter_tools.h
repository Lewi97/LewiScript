#pragma once

#include <concepts>


namespace le::iters
{
    /* advance iterator while a condition is being met */
    template<typename _Iterator> inline auto
        advance_while(_Iterator begin, _Iterator end, std::predicate<typename _Iterator::value_type> auto predicate)
        -> _Iterator
    {
        while (begin != end and predicate(*begin))
        {
            begin++;
        }

        return begin;
    }
}

