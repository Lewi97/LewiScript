#pragma once

#include <concepts>

#include "Builtin.h"

namespace le
{
    //template<typename _This, size_t _size>
    //    //requires(std::derived_from<_This, RuntimeValue>)
    //struct MemberFunctionStorage
    //{
    //    using Id = unsigned;
    //    using Args = std::vector<LeObject>;
    //    using MemberFunction = LeObject(*)(_This&, Args& args);
    //    using Container = std::array<MemberFunction, _size>;

    //    MemberFunctionStorage() = default;

    //    auto add(Id id, MemberFunction func) -> void
    //    {
    //        if (id > _size)
    //            ferr::index_out_of_range(id);
    //        functions[id] = func;
    //    }

    //    auto call(_This& this_, Id id, Args& args) -> LeObject
    //    {
    //        if (id > _size)
    //            ferr::index_out_of_range(id);
    //        if (functions.at(id) == nullptr)
    //            ferr::make_exception("Tried to call non existing member function");

    //        return functions.at(id)(*this_, args);
    //    }
    //private:
    //    Container functions{};
    //};

    /*(TODO) Some way to either allow member functions to not be stored in variables or keep alive the parent */
    template<typename _This>
    struct MemberFunction
        : RuntimeValue
    {
        using Function = LeObject(*)(_This&, std::vector<LeObject>& args);
        MemberFunction(_This& object, Function func)
            : _this(object)
            , _function(func)
        {}

        auto call(std::vector<LeObject>& args) -> LeObject override
        {
            return _function(_this, args);
        }
    private:
        _This& _this{};
        Function _function{};
    };
}

