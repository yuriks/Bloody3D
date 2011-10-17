#ifndef BLOODY3D_MATRIXTRANSFORM
#define BLOODY3D_MATRIXTRANSFORM

#include "Heatwave.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"

namespace math {

namespace mat_transform {

extern const float mat_identity[];

HW_FORCE_INLINE const mat4 translate(const vec3& v)
{
	mat4 m(mat_identity);

	// TODO: Optimize
	m.rows[0].setW(v.getX());
	m.rows[1].setW(v.getY());
	m.rows[2].setW(v.getZ());

	return m;
}

HW_FORCE_INLINE const mat3x4 translate3x4(const vec3& v)
{
	mat3x4 m(mat_identity);

	// TODO: Optimize
	m.rows[0].setW(v.getX());
	m.rows[1].setW(v.getY());
	m.rows[2].setW(v.getZ());

	return m;
}

HW_FORCE_INLINE const mat3x4 scale3x4(float scale)
{
	mat3x4 m;

	m.rows[0] = vec4(scale, 0.f,   0.f,   0.f);
	m.rows[1] = vec4(0.f,   scale, 0.f,   0.f);
	m.rows[2] = vec4(0.f,   0.f,   scale, 0.f);

	return m;
}

HW_FORCE_INLINE const mat4 scale(float scale)
{
	return padMat3x4(scale3x4(scale));
}

HW_FORCE_INLINE const mat3x4 scale3x4(const vec3& scale)
{
	mat3x4 m;

	m.rows[0] = vec4(scale.getX(), 0.f,          0.f,          0.f);
	m.rows[1] = vec4(0.f,          scale.getY(), 0.f,          0.f);
	m.rows[2] = vec4(0.f,          0.f,          scale.getZ(), 0.f);

	return m;
}

HW_FORCE_INLINE const mat4 scale(const vec3& scale)
{
	return padMat3x4(scale3x4(scale));
}

mat3x4 rotate(const vec3& axis, float angle);

// Projections
mat4 orthographic_proj(float left, float right, float bottom, float top, float z_near, float z_far);
mat4 frustrum_proj(float half_width, float half_height, float z_near, float z_far);
mat4 perspective_proj(float vfov, float aspect, float z_near, float z_far);

// Cameras
mat3x4 look_at(const vec3& up, const vec3& camera, const vec3& target);

} // namespace mat_transform

} // namespace math

#endif // BLOODY3D_MATRIXTRANSFORM
