#include "reflection.hpp"
#include "FieldAstDeserializer.hpp"

#include <algorithm>
#include <cassert>

void TypeDatabase::registerType(const TypeEntry& entry) {
	auto lower = std::lower_bound(cbegin(types), cend(types), entry.name_hash, [](const TypeEntry& e, StrHash hash) {
		return e.name_hash < hash;
	});

	// Ensure no duplicated types or hash collisions
	assert(lower == cend(types) || lower->name_hash != entry.name_hash);

	types.insert(lower, entry);
}

const TypeEntry* TypeDatabase::getType(StrHash name_hash) {
	auto lower = std::lower_bound(cbegin(types), cend(types), name_hash, [](const TypeEntry& e, StrHash hash) {
		return e.name_hash < hash;
	});

	if (lower != cend(types) && lower->name_hash == name_hash) {
		return &*lower;
	} else {
		return nullptr;
	}
}
