#pragma once

#include "util/StringHash.hpp"
#include "util/Handle.hpp"

#include <vector>

struct FieldAstDeserializer;

struct TypeEntry {
	StrHash name_hash;
	const char* name;
	ptr<Handle(FieldAstDeserializer&)> ast_deserializator;
};

struct TypeDatabase {
	void registerType(const TypeEntry& entry);
	const TypeEntry* getType(StrHash name_hash);

private:
	std::vector<TypeEntry> types;
};
