#pragma once

#include "util/Handle.hpp"
#include <unordered_map>
#include <string>
#include "math/vec.hpp"

struct Scene;
struct AstInstance;
struct ParseAst;

struct SceneAstDeserializer {
	Scene* scene;
	std::unordered_map<std::string, Handle> handle_map;

	Handle deserializeObject(const AstInstance& instance);
	void deserializeScene(const ParseAst& ast);
};
