#pragma once

#include "GlobalState.h"
#include "String.h"
#include "hashing.h"
#include "CPPLeFunction.h"

#include <unordered_map>
#include <Windows.h>

namespace le
{
    struct DllModule :
        public RuntimeValue
    {
        explicit DllModule(StringView view)
        {
            load(view);
            type = Type::Module;
        }

        using Map = std::unordered_map<hash_t, std::shared_ptr<ImportedFunction>>;

        HMODULE handle{};
        String mod_name{};
        Map functions{};

        auto type_name() -> String override
        {
            return "DllModule";
        }

        auto make_string() -> String override
        {
            return std::format("Dll Module '{}'", mod_name);
        }

        auto load(StringView module_name) -> void
        {
            handle = LoadLibraryA(module_name.data());

            if (handle == NULL)
                throw(ferr::make_exception(std::format("Failed to load dll module '{}'", module_name)));

            mod_name = module_name;
        }

        auto member_access(LeObject self, const String& member) -> LeObject override
        {
            auto hash = hashing::Hasher::hash(member);
            if (auto itr = functions.find(hash); itr != functions.end())
            {
                return itr->second;
            }

            auto f = GetProcAddress(handle, member.data());
            if (f == NULL)
                throw(ferr::make_exception(std::format("Failed to '{}' from module '{}'", member, mod_name)));

            return functions[hash] = global::mem->emplace<ImportedFunction>(reinterpret_cast<FFI_FUNC>(f));
        }
    };
}
