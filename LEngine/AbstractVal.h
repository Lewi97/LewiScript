#pragma once

#include "format_errs.h"

#include <array>
#include <concepts>

namespace le
{
    class /*alignas(128)*/ AbstractType
    {
    protected:
        constexpr static inline auto _size = 64ull;
        enum CopyState : size_t { NotCopyAble = 1 };

        typedef void(*Destructor)(void*);
        typedef std::array<std::byte, _size>(*CopyCtor)(const void*);

        alignas(64) std::array<std::byte, _size> _object{};
        Destructor _destructor{ nullptr };
        CopyCtor _copy_ctor{ nullptr };
        const type_info* _current_type = nullptr;

        auto destroy_current()
        {
            if (_destructor)
                _destructor(_object.data());
        }

        template<typename _Type>
        static auto copy_as_bytes(const _Type& other) -> std::array<std::byte, _size>
            requires std::is_copy_constructible<_Type>::value
    {
        auto container = std::array<std::byte, _size>{};
        static_assert(sizeof(_Type) <= _size);
        ::new (container.data()) _Type(other);
        return container;
    }

        template<typename _Type> static auto make_destructor()
    {
        return [](void* ptr) { static_cast<_Type*>(ptr)->~_Type(); };
    }
    template<typename _Type> auto make_copy_ctor()
    {
        if constexpr (std::copy_constructible<_Type> and not std::is_trivially_copyable<_Type>::value)
        {
            _copy_ctor = [](const void* ptr)->std::array<std::byte, _size>
            {
                return copy_as_bytes<_Type>(*static_cast<const _Type*>(ptr));
            };
        }
        else if constexpr (not std::copy_constructible<_Type>)
        {
            _copy_ctor = (CopyCtor)(CopyState::NotCopyAble);
        }
        else
        {
            _copy_ctor = (CopyCtor)(nullptr);
        }
    }

    auto& copy_type(const AbstractType& other)
    {
        if (not other.copyable())
            throw(ferr::make_exception("Tried to copy non copyable type"));
        destroy_current();
        _destructor = other._destructor;
        _current_type = other._current_type;
        if (other._copy_ctor)
        {
            _copy_ctor = other._copy_ctor;
            _object = _copy_ctor((void*)other._object.data());
        }
        else
        {
            _object = other._object;
        }
        return *this;
    }
    public:
        AbstractType() = default;

        template<typename _Type, typename ... _Args>
        explicit AbstractType(_Type*, _Args&&... args)
            requires std::constructible_from<_Type, _Args...> and (not std::same_as<AbstractType, _Type>)
        {
            construct<std::remove_reference_t<_Type>>(std::forward<_Args>(args)...);
        }

        template<typename _Type>
        explicit AbstractType(_Type&& type)
            requires (not std::same_as<AbstractType, std::remove_reference_t<_Type>>)
        {
            construct<std::remove_reference_t<_Type>>(std::forward<_Type>(type));
        }

        AbstractType(const AbstractType& other)
        {
            copy_type(other);
        }

        auto& operator=(const AbstractType& other)
        {
            return copy_type(other);
        }

        template<typename _Type>
        auto& operator=(_Type&& other)
            requires (not std::same_as<AbstractType, std::remove_reference_t<_Type>>)
        {
            construct<std::remove_reference_t<_Type>>(std::forward<_Type>(other));
            return *this;
        }

        /*auto copy() -> AbstractType { return copy_type(*this); }*/

        template<typename _Type, typename ... _Args>
        auto construct(_Args&&... args)
            requires (std::constructible_from<_Type, _Args...> and (not std::same_as<AbstractType, std::remove_reference_t<_Type>>))
        {
            static_assert(sizeof(_Type) <= _size);
            _current_type = &typeid(_Type);
            make_copy_ctor<std::remove_reference_t<_Type>>();
            _destructor = make_destructor<std::remove_reference_t<_Type>>();
            ::new (_object.data()) _Type(std::forward<_Args>(args)...);
        }

        AbstractType(AbstractType&& other) noexcept(true)
        {
            destroy_current();
            _object = other._object;
            _copy_ctor = other._copy_ctor;
            _destructor = other._destructor;
            _current_type = other.held_type();
            other._destructor = nullptr;
            other._copy_ctor = nullptr;
        }

        ~AbstractType() { destroy_current(); }

        auto object() { return _object.data(); }
        auto held_type() -> decltype(_current_type) { return _current_type; }

        template<typename _Type> auto get() -> _Type*
        {
            if (&typeid(_Type) != held_type())
                throw(ferr::make_exception("Bad get call, held type != requested type"));
            return reinterpret_cast<_Type*>(_object.data());
        }

        template<typename _Type> auto get_default(_Type* default_ = nullptr) -> _Type*
        {
            if (&typeid(_Type) == held_type())
                return reinterpret_cast<_Type*>(_object.data());
            else
                return default_;
        }

        template<typename _Type> auto get_no_throw() noexcept(true) -> _Type*
        {
            return reinterpret_cast<_Type*>(_object.data());
        }

        template<typename _Type> auto get_no_throw() const noexcept(true) -> _Type*
        {
            return reinterpret_cast<const _Type*>(_object.data());
        }

        auto copyable() const -> bool { return _copy_ctor != reinterpret_cast<CopyCtor>(CopyState::NotCopyAble); }
        auto empty() const -> bool { return _current_type == nullptr; }
    };
}
