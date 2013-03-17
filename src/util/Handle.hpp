#pragma once

#include "Heatwave.hpp"
#include <cstdint>

struct Handle {
	size_t index;
	u32 generation;

	Handle()
		: index(SIZE_MAX), generation(-1)
	{}

	Handle(size_t index, u32 generation)
		: index(index), generation(generation)
	{}

	bool operator ==(const Handle& o) const {
		return index == o.index && generation == o.generation;
	}

	bool operator !=(const Handle& o) const {
		return !(*this == o);
	}
};
