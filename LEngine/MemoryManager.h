#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <concepts>
#include <algorithm>

namespace le
{
	/*
	* Known Issues:
	* * FATAL: When vector holding pools reallocates, all existing objects in the wild will point to invalid pools.
	* * FATAL: Need a way to enforce destructing all existing objects before killing this. (Allocating pools on heap)
	*/
	class MemoryManager
	{
	public:
		template<typename _T>
		using Pointer = std::shared_ptr<_T>;
		inline static constexpr auto pool_memory_size = 4096ull; /* 4 kb */
	protected:
		struct RawMemory final
		{
			size_t size{};
			std::byte* data{};
			RawMemory() = default;
			explicit RawMemory(size_t size_)
				: size(size_)
				, data((std::byte*)malloc(size_))
			{}
			RawMemory(const RawMemory&) = delete;
			auto operator=(const RawMemory&) = delete;
			RawMemory(RawMemory&& other) noexcept
				: size(other.size)
				, data(other.data)
			{
				other.data = nullptr;
				other.size = 0ull;
			}
			~RawMemory() { delete data; }
		};

		struct Pool final
		{
			using MemBlock = std::byte*;

			explicit Pool(size_t object_size)
				: memory(pool_memory_size)
				, block_size(object_size)
				, max_size(pool_memory_size / object_size)
			{
				init();
			}
			Pool(const Pool&) = delete;
			Pool(Pool&& other) noexcept
				: memory(std::move(other.memory))
				, block_size(other.block_size)
				, next_free_block(other.next_free_block)
				, max_size(other.max_size)
				, current_size(other.current_size)
			{}

			size_t block_size{};
			RawMemory memory{};
			MemBlock next_free_block{ nullptr };
			size_t max_size{};
			size_t current_size{};

			/* DO NOT USE AS ITERATOR */ auto _begin() const -> std::byte* { return memory.data; }
			/* DO NOT USE AS ITERATOR */ auto _end() const -> std::byte* { return memory.data + memory.size; }
			/* @return Nullptr if obj does not belong to this pool */
			auto owns(void* obj) const -> bool
			{
				return (obj >= _begin() and obj < _end());
			}

			/* Doesn't check if it own's the object */
			auto _free_block_no_check(void* obj) -> void
			{
				if (empty()) return;
				memset(obj, 0, block_size); /* Debug */
				MemBlock lastfree = next_free_block;
				*(MemBlock*)obj = lastfree; /* Make current obj point to this next block */
				next_free_block = (MemBlock)obj;
				current_size--;
			}

			auto full() -> bool { return current_size >= max_size; }
			auto empty() -> bool { return current_size == 0ull; }

			auto init() -> void
			{
				memset(memory.data, 0, memory.size);
				next_free_block = memory.data;
				/*							 Avoid writing out of bounds address to end;                    */
				for (auto itr = memory.data; itr < memory.data + memory.size - block_size; itr += block_size)
				{
					auto block = (MemBlock)itr;
					*(MemBlock*)block = (MemBlock)(itr + block_size);
				}
			}

			template<typename _Type, typename ... _Args> auto emplace(_Args&&... args) -> _Type*
			{
				if (full()) throw(std::exception("Tried to emplace an object in full pool"));
				constexpr auto size = sizeof(_Type);
				if (size > block_size) throw(std::exception("Tried to allocate object greater than max object size"));
				auto* block = next_free_block;
				next_free_block = *(MemBlock*)block; /* Extract the pointer held by the memory block by casting it to a pointer to pointer */
				::new (block) _Type(std::forward<_Args>(args)...);
				current_size++;
				return (_Type*)block;
			}
		};

		/*
		* To be provided to a shared_ptr so that it can remove the data within a pool
		*/
		template<typename _T> struct Deleter final
		{
			Pool* owner{};

			explicit Deleter(Pool& pool) : owner(&pool) {}

			auto operator()(_T* obj) -> void
			{
				if constexpr (not std::is_trivially_destructible<_T>::value)
				{
					std::destroy_at<_T>(obj);
				}
				/* If obj does not belong to owner we got bigger problems */
				owner->_free_block_no_check(obj);
			}
		};
	protected:
		std::vector<Pool> _pools{};
		constexpr static auto smallest_object_size = 8;

		auto get_pool_of_size(size_t object_size) -> Pool*
		{
			for (auto& p : _pools)
			{
				if (not p.full() and p.block_size == object_size)
					return &p;
			}
			return nullptr;
		}

		constexpr static auto align_to_nearest_multiple(size_t size) -> size_t
		{
			auto arr = std::array<size_t, 8>{};
			std::generate(arr.begin(), arr.end(), [i = 0ull]() mutable { return (i++) * 8 + 8; });
			for (const auto a : arr)
			{
				if (size <= a) return a;
			}
			return size;
		}

		auto make_pool(size_t object_size) -> Pool&
		{
			return _pools.emplace_back(object_size);
		}
	public:
		MemoryManager()
		{
			_pools.reserve(32); /* Should keep us well till i fix this issue */
		}

		auto find_pool(void* object) -> Pool*
		{
			for (auto& pool : _pools)
				if (pool.owns(object))
					return &pool;
			return nullptr;
		}

		template<typename _Type, typename... _Args>
		auto emplace(_Args&&... args) -> auto
		{
			constexpr auto size = align_to_nearest_multiple(sizeof(_Type));
			auto pool = get_pool_of_size(size);

			if ((not pool) or (pool->full()))
			{
				pool = &make_pool(size);
			}

			auto ptr = pool->emplace<_Type>(std::forward<_Args>(args)...);
			return std::shared_ptr<_Type>(ptr, Deleter<_Type>(*pool));
		}

		auto free_block(auto block) -> void
			requires std::is_pointer<decltype(block)>::value
	{
		if (not block) return;
		if (auto pool = find_pool((void*)block))
			pool->_free_block_no_check((void*)block);
	}

		/* Tries to find the object in its pools to free it */
		auto free_object(auto object) -> void
		requires std::is_pointer<decltype(object)>::value
	{
		if (auto pool = find_pool((void*)object))
			pool->_free_block_no_check((void*)object);
	}

		auto highlight_free_spots(const Pool& p, std::ostream& out) const -> void
	{
		const auto max_i = 10; auto i = 0;
		for (auto itr = p.next_free_block; (Pool::MemBlock*)itr && i != max_i; (itr = *(Pool::MemBlock*)itr), i++)
		{
			out << itr << " -> ";
		}
		out << "End\n";
	}

	auto visualize_pool(const Pool& p, std::ostream& out) const -> void
	{
		out << '[';
		auto step = p.block_size;
		auto end = p.memory.data + p.memory.size;
		for (auto mem = p.memory.data; mem != end; mem += step)
		{
			auto block = *(Pool::MemBlock*)mem;
			/*
			* Not secure but incase the first 8 bytes are equal to a compatible address
			* we can be somewhat certain the object is free as it is pointing to another free object
			*/
			if (p.owns(block) or not block)
				out << '_';
			else if (mem == end - step)
			{
				if ((Pool::MemBlock*)block)
					out << '#';
				else
					out << '_';
			}
			else
				out << '#';
		}
		out << ']';
	}
	};
}
