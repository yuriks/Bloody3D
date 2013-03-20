#include "SceneReader.hpp"

#include "Heatwave.hpp"
#include "Engine.hpp"
#include "scene/Scene.hpp"
#include "ODLParse.hpp"
#include "ODLAst.hpp"
#include "util/Handle.hpp"
#include "util/ObjectPool.hpp"
#include "math/vec.hpp"
#include "math/Quaternion.hpp"
#include "util/StringHash.hpp"
#include "TypeRegistry.hpp"
#include "FieldAstDeserializer.hpp"
#include <unordered_map>

namespace serialization {

Handle reflectionObjectDispatch(std::string& str, FieldAstDeserializer& reflector) {
	const char* str_cstr = str.c_str();
	u32 str_hash = util::fnv_hash_runtime(str_cstr);

	TypeEntry* type_entry = std::find_if(type_registry, type_registry_end,
		[str_cstr, str_hash](const TypeEntry& entry) {
			return entry.name_hash == str_hash && std::strcmp(entry.name, str_cstr) == 0;
		});

	assert(type_entry != nullptr);
	return type_entry->astDeserializator(reflector);
}

Handle SceneAstDeserializer::deserializeObject(const AstInstance& instance) {
	std::string type_name(instance.type_name.begin, instance.type_name.end);

	FieldAstDeserializer field_reader;
	field_reader.scene_reader = this;
	field_reader.fields = instance.fields;
	Handle handle = reflectionObjectDispatch(type_name, field_reader);

	if (instance.handle_name.begin != nullptr) {
		std::string handle_name(instance.handle_name.begin, instance.handle_name.end);
		handle_map.insert(std::make_pair(std::move(handle_name), handle));
	}
	return handle;
}

void SceneAstDeserializer::deserializeScene(const ParseAst& ast) {
	AstInstance* instance = ast.instances;
	while (instance != nullptr) {
		deserializeObject(*instance);
		instance = instance->next;
	}
}

}

void testParse(scene::Scene& scene) {
	std::ifstream f("level_test.txt");
	serialization::InputBuffer in(&f);

	serialization::ParseAst ast;

	serialization::parseRoot(in, ast);

	if (ast.fail_message) {
		std::cerr << "Parsing failed:\n" << ast.fail_line << ": " << ast.fail_message << " Got '" << ast.fail_char << "'.\n";
	} else {
		serialization::SceneAstDeserializer scene_reader;
		scene_reader.scene = &scene;
		scene_reader.deserializeScene(ast);
	}
}
