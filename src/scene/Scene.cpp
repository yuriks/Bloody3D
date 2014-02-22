#include "Scene.hpp"

#include "math/MatrixTransform.hpp"
#include "math/misc.hpp"

namespace scene {

struct BufferSpec {
	gl::Texture* tex;
	GLenum internal_format;
	GLenum format;
	GLenum type;
	GLenum attachment;
};

void createBuffers(int width, int height, const BufferSpec* specs, size_t spec_n) {
	for (size_t i = 0; i < spec_n; ++i) {
		specs[i].tex->bind(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, specs[i].internal_format, width, height, 0,
			specs[i].format, specs[i].type, nullptr);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void attachBuffers(gl::Framebuffer& fbo, const BufferSpec* specs, size_t spec_n) {
	fbo.bind(GL_FRAMEBUFFER);
	for (size_t i = 0; i < spec_n; ++i) {
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, specs[i].attachment, GL_TEXTURE_2D, *specs[i].tex, 0);
	}

	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Incomplete framebuffer." << std::endl;
}

void GBufferSet::initialize(int width, int height) {
	std::array<BufferSpec, 3> buffers = {{
		// tex         internal_format      format            type                  attachment
		{&depth_tex,   GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, GL_DEPTH_STENCIL_ATTACHMENT},
		{&diffuse_tex, GL_SRGB8_ALPHA8,     GL_RGBA,          GL_FLOAT,             GL_COLOR_ATTACHMENT0},
		{&normal_tex,  GL_RGB10_A2,         GL_RGBA,          GL_FLOAT,             GL_COLOR_ATTACHMENT1}
	}};

	createBuffers(width, height, buffers.data(), buffers.size());
	attachBuffers(fbo, buffers.data(), buffers.size());

	static const GLenum render_targets[2] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};
	glDrawBuffers(2, render_targets);
}

void ShadingBufferSet::initialize(int width, int height, gl::Texture& depth_tex) {
	std::array<BufferSpec, 2> buffers = {{
		// tex       internal_format    format   type      attachment
		{&depth_tex, 0,                 0,       0,        GL_DEPTH_ATTACHMENT},
		{&accum_tex, GL_R11F_G11F_B10F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0}
	}};

	createBuffers(width, height, &buffers[1], 1);
	attachBuffers(fbo, buffers.data(), buffers.size());

	static const GLenum render_targets[1] = {
		GL_COLOR_ATTACHMENT0
	};
	glDrawBuffers(1, render_targets);
}

void renderGeometry(
	const Scene& scene,
	const math::mat4& world2view_mat,
	const math::mat4* model2world_mats,
	GBufferSet& buffers,
	const RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms)
{
	const Engine& engine = *scene.engine;
	auto& gpu_meshes = engine.gpu_meshes;
	auto& instances = scene.mesh_instances;

	std::vector<Handle> sorted_instances(instances.pool.size());
	for (unsigned int i = 0; i < sorted_instances.size(); ++i) {
		sorted_instances[i] = instances.makeHandle(i);
	}

	// Sort instances by material, material_params, instance
	std::sort(sorted_instances.begin(), sorted_instances.end(), [&](Handle a, Handle b) -> bool {
		size_t mat_id_a = gpu_meshes[instances[a]->mesh_id]->material_id.index;
		size_t mat_id_b = gpu_meshes[instances[b]->mesh_id]->material_id.index;
		if (mat_id_a == mat_id_b) {
			size_t matpar_id_a = instances[a]->material_opts.index;
			size_t matpar_id_b = instances[b]->material_opts.index;
			if (matpar_id_a == matpar_id_b) {
				return a.index < b.index;
			} else {
				return matpar_id_a < matpar_id_b;
			}
		} else {
			return mat_id_a < mat_id_b;
		}
	});

	Handle cur_material_id;
	int mtl_options_size = 0;

	Handle cur_mesh_id;
	Handle cur_material_params_id;

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
			cur_mesh_id = instance->mesh_id;

			if (cur_material_id != mesh->material_id) {
				// Load material
				cur_material_id = mesh->material_id;

				const Material* mtl = engine.materials[cur_material_id];
				mtl->shader_program.use();
				mtl_options_size = mtl->options_size;
			}

			mesh->vao.bind();
			mesh->ibo.bind(GL_ELEMENT_ARRAY_BUFFER);
		}

		if (cur_material_params_id != instance->material_opts) {
			cur_material_params_id = instance->material_opts;
			const MaterialOptions* params = scene.material_options[cur_material_params_id];

			render_context.material_ubo.bind(GL_UNIFORM_BUFFER);
			glBufferData(GL_UNIFORM_BUFFER, mtl_options_size, params->uniforms.get(), GL_STREAM_DRAW);

			for (int t = 0; t < 4; ++t) {
				const Texture* tex = engine.textures[params->texture_ids[t]];
				if (tex != nullptr) {
					glActiveTexture(GL_TEXTURE0 + t);
					tex->gl_tex.bind(GL_TEXTURE_2D);
				}
			}
			glFlush();
		}

		// TODO: Instancing
		math::mat4 model2world_mat = model2world_mats[scene.transforms.getPoolIndex(instance->transform)];
		math::mat4 model2view_mat = world2view_mat * model2world_mat;

		render_context.system_ubo.bind(GL_UNIFORM_BUFFER);
		SystemUniformBlock sys_uniforms_copy = sys_uniforms;
		sys_uniforms_copy.view_model_mat = model2view_mat;
		glBufferData(GL_UNIFORM_BUFFER, sizeof(SystemUniformBlock), &sys_uniforms_copy, GL_STREAM_DRAW);

		glDrawElements(GL_TRIANGLES, mesh->indices_count, mesh->indices_type, 0);
		glFlush(); // BUG: Work around for intel bug: glBufferData is ignored
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
