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
#include "FieldAstDeserializer.hpp"
#include <unordered_map>
#include "objects.hpp"

Handle reflectionObjectDispatch(std::string& str, FieldAstDeserializer& reflector) {
	const char* str_cstr = str.c_str();
	u32 str_hash = fnv_hash_runtime(str_cstr);

	const TypeEntry* type_entry = type_database.getType(str_hash);

	assert(type_entry != nullptr);
	return type_entry->ast_deserializator(reflector);
}

Handle SceneAstDeserializer::deserializeObject(const AstInstance& instance) {
	std::string type_name(instance.type_name.begin, instance.type_name.end);

	FieldAstDeserializer field_reader;
	field_reader.scene_reader = this;
	field_reader.fields = instance.fields;
	Handle handle = reflectionObjectDispatch(type_name, field_reader);

	if (instance.handle_name.begin != nullptr) {
		std::string handle_name(instance.handle_name.begin, instance.handle_name.end);
		if (instance.handle_name.create) {
			scene->named_handles.insert(std::make_pair(handle_name, handle));
		}
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

void testParse(Scene& scene) {
	std::ifstream f("level.odl");
	InputBuffer in(&f);

	ParseAst ast;

	parseRoot(in, ast);

	if (ast.fail_message) {
		std::cerr << "Parsing failed:\n" << ast.fail_line << ": " << ast.fail_message << " Got '" << ast.fail_char << "'.\n";
	} else {
		SceneAstDeserializer scene_reader;

		SerializationPool<Transform> tmp0(&scene.transforms);
		SerializationPool<DirectionalLight> tmp1(&scene.lights_dir);
		SerializationPool<OmniLight> tmp2(&scene.lights_omni);
		SerializationPool<SpotLight> tmp3(&scene.lights_spot);
		SerializationPool<Camera> tmp4(&scene.cameras);
		SerializationPool<MeshInstance> tmp5(&scene.mesh_instances);
		SerializationPool<MaterialOptions> tmp6(&scene.material_options);
		SerializationPool<Texture> tmp7(&scene.engine->textures);
		SerializationPool<GPUMesh> tmp8(&scene.engine->gpu_meshes);

		scene_reader.scene = &scene;
		scene_reader.handle_map = scene.named_handles;
		scene_reader.deserializeScene(ast);
	}
}
