#include "Scene.hpp"

#include "math/MatrixTransform.hpp"
#include "math/misc.hpp"

namespace scene {

void GBufferSet::initialize(int width, int height) {
	depth_tex.bind(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

	diffuse_tex.bind(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

	normal_tex.bind(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB10_A2, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

	glBindTexture(GL_TEXTURE_2D, 0);

	fbo.bind(GL_FRAMEBUFFER);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_tex, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffuse_tex, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_tex, 0);

	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Incomplete framebuffer." << std::endl;

	static const GLenum render_targets[2] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};
	glDrawBuffers(2, render_targets);
}

void ShadingBufferSet::initialize(int width, int height, gl::Texture& depth_tex) {
	accum_tex.bind(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

	glBindTexture(GL_TEXTURE_2D, 0);

	fbo.bind(GL_FRAMEBUFFER);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_tex, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accum_tex, 0);

	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Incomplete framebuffer." << std::endl;

	static const GLenum render_targets[1] = {
		GL_COLOR_ATTACHMENT0
	};
	glDrawBuffers(1, render_targets);
}

int Scene::addMaterial(Material&& mat) {
	int new_id = material_list.size();
	material_list.push_back(std::move(mat));
	return new_id;
}

int Scene::addMesh(GPUMesh&& mesh) {
	int new_id = gpu_meshes.size();
	gpu_meshes.push_back(std::move(mesh));
	mesh_instances.resize(gpu_meshes.size());
	return new_id;
}

MeshInstanceHandle Scene::newInstance(int mesh_id) {
	MeshInstanceHandle handle;
	handle.mesh_id = mesh_id;
	handle.instance_id = mesh_instances[mesh_id].size();

	mesh_instances[mesh_id].push_back(MeshInstance());
	return handle;
}

void renderGeometry(
	const Scene& scene,
	const math::mat4& world2view_mat,
	GBufferSet& buffers,
	RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms)
{
	std::vector<unsigned int> gpumesh_indices(scene.gpu_meshes.size());
	for (unsigned int i = 0; i < gpumesh_indices.size(); ++i)
		gpumesh_indices[i] = i;

	std::sort(gpumesh_indices.begin(), gpumesh_indices.end(), [&](unsigned int a, unsigned int b) -> bool {
		int mat_id_a = scene.gpu_meshes[a].material_id;
		int mat_id_b = scene.gpu_meshes[b].material_id;
		if (mat_id_a == mat_id_b) {
			return a < b;
		} else {
			return mat_id_a < mat_id_b;
		}
	});

	int cur_material_id = -1;
	int mtl_options_size = 0;

	render_context.system_ubo.bind(GL_UNIFORM_BUFFER);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SystemUniformBlock), &sys_uniforms, GL_STREAM_DRAW);

	buffers.fbo.bind(GL_DRAW_FRAMEBUFFER);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	glDisable(GL_BLEND);

	math::vec4 clear_albedo = {0, 0, 0, 0};
	math::vec4 clear_normal = {0.5f, 0.5f, 0, 0};
	float clear_depth = 1.f;
	glClearBufferfv(GL_COLOR, 0, clear_albedo.data);
	glClearBufferfv(GL_COLOR, 1, clear_normal.data);
	glClearBufferfv(GL_DEPTH, 0, &clear_depth);
	glEnable(GL_FRAMEBUFFER_SRGB);

	for (unsigned int i = 0; i < scene.gpu_meshes.size(); ++i) {
		const GPUMesh& mesh = scene.gpu_meshes[gpumesh_indices[i]];

		if (cur_material_id != mesh.material_id) {
			// Load material
			cur_material_id = mesh.material_id;

			scene.material_list[cur_material_id].shader_program.use();
			mtl_options_size = scene.material_list[cur_material_id].options_size;
		}

		// Load mesh
		render_context.material_ubo.bind(GL_UNIFORM_BUFFER);
		glBufferData(GL_UNIFORM_BUFFER, mtl_options_size, mesh.material_options.uniforms.get(), GL_STREAM_DRAW);

		for (int t = 0; t < 4; ++t) {
			const gl::Texture* tex = scene.tex_manager.lookupTexture(mesh.material_options.texture_ids[t]);
			if (tex != nullptr) {
				glActiveTexture(GL_TEXTURE0 + t);
				tex->bind(GL_TEXTURE_2D);
			}
		}

		mesh.vao.bind();
		mesh.ibo.bind(GL_ELEMENT_ARRAY_BUFFER);

		const auto& inst_list = scene.mesh_instances[gpumesh_indices[i]];
		
		util::AlignedVector<math::mat4> model2view_mats;
		model2view_mats.resize(inst_list.size());
		for (unsigned int j = 0; j < model2view_mats.size(); ++j) {
			math::mat4 model2world_mat = calcTransformMtx(inst_list[j].t);
			model2view_mats[j] = world2view_mat * model2world_mat;
		}

		// TODO: Instancing
		render_context.system_ubo.bind(GL_UNIFORM_BUFFER);
		for (unsigned int j = 0; j < model2view_mats.size(); ++j) {
			SystemUniformBlock sys_uniforms_copy = sys_uniforms;
			sys_uniforms_copy.view_model_mat = model2view_mats[j];
			glBufferData(GL_UNIFORM_BUFFER, sizeof(SystemUniformBlock), &sys_uniforms_copy, GL_STREAM_DRAW);

			glDrawElements(GL_TRIANGLES, mesh.indices_count, mesh.indices_type, 0);
		}
	}

	for (int t = 0; t < 4; ++t) {
		glActiveTexture(GL_TEXTURE0 + t);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glDisable(GL_FRAMEBUFFER_SRGB);
}

void bindGBufferTextures(GBufferSet& gbuffer) {
	glActiveTexture(GL_TEXTURE0);
	gbuffer.depth_tex.bind(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	gbuffer.diffuse_tex.bind(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE2);
	gbuffer.normal_tex.bind(GL_TEXTURE_2D);
}

void transformDirectionalLights(
	const std::vector<DirectionalLight>& in_lights,
	std::vector<GPUDirectionalLight>& out_lights,
	const math::mat4& world2view_mat)
{
	out_lights.resize(in_lights.size());
	for (size_t i = 0; i < in_lights.size(); ++i) {
		const math::mat4 model2view_mat = world2view_mat * calcTransformMtx(in_lights[i].t);
		out_lights[i].direction = -math::mvec3(model2view_mat * math::mvec4(math::vec3_z, 0.0f));
		out_lights[i].color = in_lights[i].color;
	}
}

// Shades a collection of directional lights to the shading buffer. Lights are in view-space.
void shadeDirectionalLights(
	const std::vector<GPUDirectionalLight>& lights,
	const Material& light_material,
	RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	light_material.shader_program.use();
	assert(light_material.options_size == sizeof(GPUDirectionalLight));

	render_context.system_ubo.bind(GL_UNIFORM_BUFFER);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SystemUniformBlock), &sys_uniforms, GL_STREAM_DRAW);

	render_context.material_ubo.bind(GL_UNIFORM_BUFFER);

	for (const GPUDirectionalLight& light : lights) {
		glBufferData(GL_UNIFORM_BUFFER, light_material.options_size, &light, GL_STREAM_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}

} // namespace scene
