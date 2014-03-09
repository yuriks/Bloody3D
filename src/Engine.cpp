#include "Engine.hpp"

#include "scene/DirectionalLight.hpp"
#include "scene/OmniLight.hpp"
#include "scene/SpotLight.hpp"

void loadEngineMaterials(Engine& engine) {
	MaterialTemplate material_template;
	material_template.attachShaders("light_directional");
	material_template.options_size = 0;
	engine.dirlight.material = engine.materials.insert(material_template.compile());
	setupDirLightVao(engine.dirlight);

	material_template.clear();
	material_template.attachShaders("light_omni");
	material_template.options_size = 0;
	engine.omnilight.material = engine.materials.insert(material_template.compile());
	setupOmniLightVao(engine.omnilight);

	material_template.clear();
	material_template.attachShaders("light_spot");
	material_template.options_size = 0;
	engine.spotlight.material = engine.materials.insert(material_template.compile());
	setupSpotLightVao(engine.spotlight);

	material_template.clear();
	material_template.attachShaders("fullscreen_triangle.vert", "tonemap.frag");
	material_template.options_size = 0;
	engine.tonemap_material = engine.materials.insert(material_template.compile());
}
