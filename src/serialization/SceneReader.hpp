#pragma once

#include "util/Handle.hpp"
#include <unordered_map>
#include <string>
#include "math/vec.hpp"

namespace scene {
	struct Scene;
}

namespace math {
	struct Quaternion;
}

namespace serialization {

struct AstInstance;
struct AstField;
struct ParseAst;
struct FieldAstDeserializer;

struct SceneAstDeserializer {
	scene::Scene* scene;
	std::unordered_map<std::string, Handle> handle_map;

	Handle deserializeObject(const AstInstance& instance);
	void deserializeScene(const ParseAst& ast);
};

}
