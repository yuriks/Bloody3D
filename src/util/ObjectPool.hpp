#pragma once

#include "Heatwave.hpp"
#include <vector>
#include <type_traits>
#include <cassert>
#include <utility>
#include "Handle.hpp"

/** Manages a pool of objects, providing persistent handles to them. */
template <typename T>
struct ObjectPool {
	size_t first_free_index; // in roster
	
	std::vector<Handle> roster; // .index is next free index for unused entries
	std::vector<std::pair<size_t, T>> pool;

	ObjectPool()
		: first_free_index(MAXSIZE_T)
	{}

	Handle insert(T object = T()) {
		// Expand roster if we're out of entries
		if (first_free_index >= roster.size()) {
			expand_roster();
		}

		// Pop head off of free list
		const size_t roster_index = first_free_index;
		first_free_index = roster[roster_index].index;

		// Point roster entry to right place and insert object
		roster[roster_index].index = pool.size();
		pool.push_back(std::make_pair(roster_index, std::move(object)));

		return Handle(roster_index, roster[roster_index].generation);
	}

	void remove(const Handle h) {
		if (!isValid(h))
			return;

		const size_t roster_index = h.index;
		const size_t pool_index = roster[roster_index].index;
		
		const size_t moved_roster_index = pool.back().first;
		const size_t moved_pool_index = pool.size() - 1;
		assert(roster[moved_roster_index].index == moved_pool_index);

		// Move last element in place of the removed one, updating roster
		roster[moved_roster_index].index = pool_index;
		pool[pool_index] = std::move(pool[moved_pool_index]);
		pool.pop_back();

		// Increment generation of removed roster entry and add it to free list
		++roster[roster_index].generation;
		roster[roster_index].index = first_free_index;
		first_free_index = roster_index;
	}

	T* operator[] (const Handle h) {
		if (isValid(h)) {
			assert(roster[h.index].index < pool.size());
			return &pool[roster[h.index].index].second;
		} else {
			return nullptr;
		}
	}

	const T* operator[] (const Handle h) const {
		if (isValid(h)) {
			assert(roster[h.index].index < pool.size());
			return &pool[roster[h.index].index].second;
		} else {
			return nullptr;
		}
	}

	bool isValid(const Handle h) const {
		return h.index < roster.size() && roster[h.index].generation == h.generation;
	}

private:
	void expand_roster() {
		const Handle new_entry(first_free_index, 0);

		first_free_index = roster.size();
		roster.push_back(new_entry);

		assert(first_free_index < roster.size());
	}
};

#if 0
void test_ObjectPool() {
	ObjectPool<float> objp;
	const auto& objp_c = objp;

	Handle h1 = objp.insert(42.0f);
	Handle h2 = objp.insert(1000.0f);

	float* f1 = objp[h1];
	assert(f1 && *f1 == 42.0f);
	const float* f2 = objp_c[h2];
	assert(f2 && *f2 == 1000.0f);

	objp.remove(h2);

	const float* f3 = objp_c[h2];
	assert(!f3);

	objp.remove(h1);

	Handle h3 = objp.insert(112233.0f);

	objp.remove(h1);

	const float* f4 = objp[h3];
	assert(f4 && *f4 == 112233.0f);

	objp.remove(h3);

	assert(objp.pool.empty());
}
#endif