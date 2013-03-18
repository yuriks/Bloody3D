#include "Transform.hpp"

#include "math/MatrixTransform.hpp"
#include "Scene.hpp"

namespace scene {

math::mat4 calcTransformMtx(const Transform& t) {
	math::mat4 tm = math::mat_transform::translate(t.pos);
	math::mat3 rm = math::matrixFromQuaternion(t.rot);
	math::mat3 sm = math::mat_transform::scale(t.scale);
	return tm * math::pad<4>(rm * sm);
}

math::mat4 calcInvTransformMtx(const Transform& t) {
	math::mat4 tm = math::mat_transform::translate(-t.pos);
	math::mat3 rm = math::matrixFromQuaternion(math::conjugate(t.rot));
	math::mat3 sm = math::mat_transform::scale(1.0f / t.scale);
	return math::pad<4>(sm * rm) * tm;
}

void calculateModel2WorldMatrices(
	const ObjectPool<Transform>& transforms,
	math::mat4* out, math::mat4* out_inverse)
{
	for (size_t i = 0, end = transforms.pool.size(); i < end; ++i) {
		out[i] = calcTransformMtx(transforms.pool[i]);
		out_inverse[i] = calcInvTransformMtx(transforms.pool[i]);
	}
}

} // namespace scene
