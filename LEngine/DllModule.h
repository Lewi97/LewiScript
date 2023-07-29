#pragma once

#include "GlobalState.h"
#include "Module.h"
#include "String.h"
#include "hashing.h"
#include "CPPLeFunction.h"

#include <unordered_map>
#include <Windows.h>

namespace le
{
    struct DllModule :
        public Module
    {
        using Module::Module;
        using Map = std::unordered_map<hash_t, std::shared_ptr<ImportedFunction>>;

        HMODULE handle{};
        String mod_name{};
        Map functions{};

        auto load(String module_name) -> LeObject override
        {
            handle = LoadLibraryA(module_name.c_str());

            if (handle == NULL)
                throw(ferr::make_exception(std::format("Failed to load module '{}'", module_name)));

            mod_name = module_name;
        }

        auto access(LeObject index) -> LeObject override
        {
            if (index->type != Type::String)
                throw(ferr::make_exception(std::format("Cannot access member of dll module with {}", to_string(index->type))));
            auto& f_name = static_cast<StringValue*>(index.get())->string;
            auto hash = hashing::Hasher::hash(f_name);
            if (auto itr = functions.find(hash); itr != functions.end())
            {
                return itr->second;
            }

            auto f = GetProcAddress(handle, f_name.c_str());
            if (f == NULL)
                throw(ferr::make_exception(std::format("Failed to '{}' from module '{}'", f_name, mod_name)));

            return functions[hash] = global::mem->emplace<ImportedFunction>(reinterpret_cast<FFI_FUNC>(f));
        }
    };
}
