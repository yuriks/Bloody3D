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

Handle Scene::newInstance(Handle mesh_id) {
	MeshInstance instance;
	instance.mesh_id = mesh_id;

	return mesh_instances.insert(instance);
}

void renderGeometry(
	const Scene& scene,
	const math::mat4& world2view_mat,
	GBufferSet& buffers,
	RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms)
{
	const Engine& engine = *scene.engine;
	auto& gpu_meshes = engine.gpu_meshes;
	auto& instances = scene.mesh_instances;

	std::vector<Handle> sorted_instances(instances.pool.size());
	for (unsigned int i = 0; i < sorted_instances.size(); ++i) {
		sorted_instances[i] = instances.makeHandle(i);
	}

	// Sort instances first by material, then by instance
	std::sort(sorted_instances.begin(), sorted_instances.end(), [&](Handle a, Handle b) -> bool {
		Handle mat_id_a = gpu_meshes[instances[a]->mesh_id]->material_id;
		Handle mat_id_b = gpu_meshes[instances[b]->mesh_id]->material_id;
		if (mat_id_a.index == mat_id_b.index) {
			return a.index < b.index;
		} else {
			return mat_id_a.index < mat_id_b.index;
		}
	});

	Handle cur_material_id;
	int mtl_options_size = 0;

	Handle cur_mesh_id;

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

	for (const Handle& instance_h : sorted_instances) {
		const MeshInstance* instance = instances[instance_h];
		const GPUMesh* mesh = gpu_meshes[instance->mesh_id];

		if (cur_mesh_id != instance->mesh_id) {
			// Load mesh

			if (cur_material_id != mesh->material_id) {
				// Load material
				cur_material_id = mesh->material_id;

				const Material* mtl = engine.materials[cur_material_id];
				mtl->shader_program.use();
				mtl_options_size = mtl->options_size;
			}

			render_context.material_ubo.bind(GL_UNIFORM_BUFFER);
			glBufferData(GL_UNIFORM_BUFFER, mtl_options_size, mesh->material_options.uniforms.get(), GL_STREAM_DRAW);

			for (int t = 0; t < 4; ++t) {
				const gl::Texture* tex = engine.texture_manager.textures[mesh->material_options.texture_ids[t]];
				if (tex != nullptr) {
					glActiveTexture(GL_TEXTURE0 + t);
					tex->bind(GL_TEXTURE_2D);
				}
			}

			mesh->vao.bind();
			mesh->ibo.bind(GL_ELEMENT_ARRAY_BUFFER);
		}

		// TODO: Instancing
		math::mat4 model2world_mat = calcTransformMtx(instance->t);
		math::mat4 model2view_mat = world2view_mat * model2world_mat;

		render_context.system_ubo.bind(GL_UNIFORM_BUFFER);
		SystemUniformBlock sys_uniforms_copy = sys_uniforms;
		sys_uniforms_copy.view_model_mat = model2view_mat;
		glBufferData(GL_UNIFORM_BUFFER, sizeof(SystemUniformBlock), &sys_uniforms_copy, GL_STREAM_DRAW);

		glDrawElements(GL_TRIANGLES, mesh->indices_count, mesh->indices_type, 0);
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

} // namespace scene
