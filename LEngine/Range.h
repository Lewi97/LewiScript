#pragma once

#include "Builtin.h"
#include "MemoryManager.h"
#include "GlobalState.h"
#include "Iterator.h"
#include "TypeFactory.h"
#include "getters.h"

namespace le
{
    struct Range : RuntimeValue
    {
        explicit Range(Number end_, Number step_ = 1.0)
            : start(0.0)
            , end(end_)
            , step(step_)
        {
            type = Type::Custom;
        }

        Range(Number start_, Number end_, Number step_ = 1.0)
            : start(start_)
            , end(end_)
            , step(step_)
        {
            type = Type::Custom;
        }

        Number start{};
        Number end{};
        Number step{};

        auto type_name() -> String override
        {
            return "Range";
        }

        auto iterator(LeObject self) -> LeObject override
        {
            auto iterator_next =
                [count = 0.0](Range& range) mutable -> LeObject
            {
                auto current = range.start + (range.step * count);
                count += 1.0;
                if (current >= range.end)
                    return make::make_null();
                else
                    return make::make_number(current);
            };

            return global::mem->emplace<Iterator<Range, decltype(iterator_next)>>(self, iterator_next);
        }
    };

    inline auto range_constructor(std::span<LeObject> args, MemoryManager& mem) -> LeObject
    {
        const auto context = "While calling Range()";

        switch (args.size())
        {
        case 1:
            return mem.emplace<Range>(getters::get_number(args[0], context));
        case 2:
            return mem.emplace<Range>(getters::get_number(args[0], context), getters::get_number(args[1], context), 1.0);
        case 3:
            return mem.emplace<Range>(getters::get_number(args[0], context), getters::get_number(args[1], context), getters::get_number(args[2], context));
        default:
            throw(ferr::too_many_arguments(args.size(), 3, "Range"));
        }
    }
}
