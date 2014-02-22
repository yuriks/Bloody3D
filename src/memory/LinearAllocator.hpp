#pragma once
#include "Heatwave.hpp"
#include <cstddef>
#include <cassert>
#include <memory>
#include <type_traits>

namespace memory {

class LinearAllocator {
public:
	LinearAllocator(u8* memory_area, size_t max_size) :
		memory_start(memory_area),
		memory_cur(memory_area),
		memory_end(memory_area + max_size)
	{
	}

	u8* allocate(size_t size) {
		u8* p = memory_cur;
		memory_cur += size;
		assert(memory_cur >= p && memory_cur < memory_end);
		return p;
	}

	template <typename T>
	T* allocate(size_t count) {
		static_assert(std::is_trivial<T>::value, "T must be trivial");

		const size_t allocation_size = count * sizeof(T);

		// Modified by std::align
		size_t remaining = memory_end - memory_cur;
		void* void_cur = static_cast<void*>(memory_cur);

		void* p = std::align(std::alignment_of<T>::value, allocation_size, void_cur, remaining);
		assert(p);

		memory_cur = static_cast<u8*>(void_cur) +allocation_size;
		return static_cast<T*>(p);
	}

	template <typename T>
	T* allocate() {
		return allocate<T>(1);
	}

	void rewind() {
		memory_cur = memory_start;
	}

private:
	u8* memory_start;
	u8* memory_cur;
	u8* memory_end;
};

}
