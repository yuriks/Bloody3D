#include "Scene.hpp"

#include "math/MatrixTransform.hpp"
#include "math/misc.hpp"

namespace scene {

void RenderBufferSet::initialize(int width, int height) {
	depth_tex.bind(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	diffuse_tex.bind(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

	normal_tex.bind(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

	fbo.bind(GL_FRAMEBUFFER);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex, 0);
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

MeshInstance& Scene::newInstance(int mesh_id) {
	mesh_instances[mesh_id].push_back(MeshInstance());
	return mesh_instances[mesh_id].back();
}

void renderGeometry(const Scene& scene, const Camera& camera, RenderBufferSet& buffers, RenderContext& render_context) {
	SystemUniformBlock sys_uniforms;
	sys_uniforms.projection_mat = math::mat_transform::perspective_proj(camera.fov, render_context.aspect_ratio, camera.clip_near, camera.clip_far);

	math::mat3x4 world2view_mat = math::concatTransform(math::mat_transform::translate3x4(-camera.pos), math::matrixFromQuaternion(math::conjugate(camera.rot)));

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

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

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
		
		util::AlignedVector<math::mat3x4> model2view_mats;
		model2view_mats.resize(inst_list.size());
		for (unsigned int j = 0; j < model2view_mats.size(); ++j) {
			const MeshInstance& inst = inst_list[j];
			math::mat3x4 t = math::mat_transform::translate3x4(math::vec3(inst.pos_scale));
			math::mat3x4 r = math::matrixFromQuaternion(inst.rot);
			math::mat3x4 s = math::mat_transform::scale3x4(inst.pos_scale.getW());
			math::mat3x4 model2world_mat = math::concatTransform(t, math::concatTransform(r, s));
			model2view_mats[j] = math::concatTransform(world2view_mat, model2world_mat);
		}

		// TODO: Instancing
		render_context.system_ubo.bind(GL_UNIFORM_BUFFER);
		for (unsigned int j = 0; j < model2view_mats.size(); ++j) {
			sys_uniforms.view_model_mat = model2view_mats[j];
			glBufferData(GL_UNIFORM_BUFFER, sizeof(SystemUniformBlock), &sys_uniforms, GL_STREAM_DRAW);

			glDrawElements(GL_TRIANGLES, mesh.indices_count, GL_UNSIGNED_SHORT, 0);
		}
	}
}

void shadeBuffers(const util::AlignedVector<Light>& lights, const Material& shading_material, RenderBufferSet& buffers, GLuint destination_fbo, RenderContext& render_context) {
	glDisable(GL_DEPTH_TEST);
	shading_material.shader_program.use();

	render_context.material_ubo.bind(GL_UNIFORM_BUFFER);
	glBufferData(GL_UNIFORM_BUFFER, shading_material.options_size, 0, GL_STREAM_DRAW);

	glActiveTexture(GL_TEXTURE0);
	buffers.diffuse_tex.bind(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	buffers.normal_tex.bind(GL_TEXTURE_2D);

	glBindFramebuffer(GL_FRAMEBUFFER, destination_fbo);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

} // namespace scene
