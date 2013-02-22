#pragma once

#include "util/noncopyable.hpp"
#include <malloc.h>
#include <cstring>
#include <cassert>

namespace util {

// WARNING: You should only use this with POD (or similarly trivial) objects. No constructors or destructors called.

template <typename T, size_t Align = 16>
class AlignedVector {
public:
	typedef T* iterator;
	typedef const T* const_iterator;

	AlignedVector() : vec_data(nullptr), vec_alloc_end(nullptr), vec_data_end(nullptr) {}

	AlignedVector(unsigned int max_size)
		: vec_data(_aligned_malloc(max_size * sizeof(T), Align)),
		vec_alloc_end(vec_data + max_size),
		vec_data_end(vec_data)
	{
		assert(vec_data != 0);
	}

	AlignedVector(const AlignedVector& o)
		: vec_data(static_cast<T*>(_aligned_malloc(o.size() * sizeof(T), Align))),
		vec_alloc_end(vec_data + o.size()),
		vec_data_end(vec_alloc_end)
	{
		assert(vec_data != 0);
		std::memcpy(vec_data, o.vec_data, o.size() * sizeof(T));
	}

	~AlignedVector() {
		_aligned_free(vec_data);
	}

	AlignedVector& operator=(const AlignedVector& o) {
		reserve(o.size());
		std::memcpy(vec_data, o.vec_data, o.size() * sizeof(T));
		vec_data_end = vec_data + o.size();

		return *this;
	}

	T& operator[](unsigned int i) {
		return vec_data[i];
	}

	const T& operator[](unsigned int i) const {
		return vec_data[i];
	}

	T* data() {
		return vec_data;
	}

	unsigned int size() const {
		return vec_data_end - vec_data;
	}

	unsigned int capacity() const {
		return vec_alloc_end - vec_data;
	}

	bool empty() const {
		return vec_data == vec_data_end;
	}

	void resize(unsigned int count) {
		reserve(count);
		vec_data_end = vec_data + count;
	}

	void reserve(unsigned int count) {
		if (capacity() < count) {
			unsigned int old_size = size();
			T* new_vec = static_cast<T*>(_aligned_malloc(count * sizeof(T), Align));
			assert(new_vec);

			std::memcpy(new_vec, vec_data, old_size * sizeof(T));

			_aligned_free(vec_data);

			vec_data = new_vec;
			vec_alloc_end = new_vec + count;
			vec_data_end = new_vec + old_size;
		}
	}

	void shrink_to_fit() {
		if (size() != capacity()) {
			unsigned int old_size = size();
			T* new_vec = _aligned_malloc(old_size * sizeof(T), Align);

			std::memcpy(new_vec, vec_data, old_size * sizeof(T));

			_aligned_free(vec_data);

			vec_data = new_vec;
			vec_alloc_end = new_vec + old_size;
			vec_data_end = new_vec + old_size;
		}
	}

	void push_back(const T& val) {
		expandStorage();
		*vec_data_end++ = val;
	}

	T& push_back() {
		expandStorage();
		return *vec_data_end++;
	}

	T& back() {
		return *(vec_data_end - 1);
	}

	const T& back() const {
		return *(vec_data_end - 1);
	}

	iterator begin() {
		return vec_data;
	}

	iterator end() {
		return vec_data_end;
	}

	const_iterator cbegin() const {
		return vec_data;
	}

	const_iterator cend() const {
		return vec_data_end;
	}

	void swap(AlignedVector& o) {
		std::swap(vec_data, o.vec_data);
		std::swap(vec_alloc_end, o.vec_alloc_end);
		std::swap(vec_data_end, o.vec_data_end);
	}

protected:
	void expandStorage() {
		if (size() == capacity())
			reserve(capacity() == 0 ? 16 : capacity() + capacity() / 2); // k = 1.5
	}

	T* vec_data;
	T* vec_alloc_end;
	T* vec_data_end;
};

template <typename T, size_t Align>
void swap(AlignedVector<T, Align>& a, AlignedVector<T, Align>& b) {
	a.swap(b);
}

} // namespace util
