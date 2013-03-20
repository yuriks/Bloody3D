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
	T* allocate() {
		static_assert(std::is_pod<T>::value, "T must be a POD");

		size_t remaining = memory_end - memory_cur;

		void* void_cur = static_cast<void*>(memory_cur);
		void* p = std::align(std::alignment_of<T>::value, sizeof(T), void_cur, remaining);
		memory_cur = static_cast<u8*>(void_cur);

		assert(p);
		return static_cast<T*>(p);
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
