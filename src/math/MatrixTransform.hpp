#ifndef BLOODY3D_MATRIXTRANSFORM
#define BLOODY3D_MATRIXTRANSFORM

#include "Heatwave.hpp"
#include "mat.hpp"
#include "vec.hpp"

HW_FORCE_INLINE const mat4 translate(const vec3& v)
{
	mat4 m = {{
		{1, 0, 0, v[0]},
		{0, 1, 0, v[1]},
		{0, 0, 1, v[2]},
		{0, 0, 0,   1 }
	}};

	return m;
}

HW_FORCE_INLINE const mat3 scale(float scale)
{
	mat3 m = {{
		{scale,   0,     0, },
		{  0,   scale,   0, },
		{  0,     0,   scale}
	}};

	return m;
}

HW_FORCE_INLINE const mat3 scale(const vec3& scale)
{
	mat3 m = {{
		{scale[0],    0,        0,   },
		{   0,     scale[1],    0,   },
		{   0,        0,     scale[2]}
	}};

	return m;
}

mat3 rotate(const vec3& axis, float angle);

// Projections
mat4 orthographic_proj(float left, float right, float bottom, float top, float z_near, float z_far);
mat4 frustrum_proj(float half_width, float half_height, float z_near, float z_far);
mat4 perspective_proj(float vfov, float aspect, float z_near, float z_far);

// Cameras
mat4 look_at(const vec3& up, const vec3& camera, const vec3& target);

#endif // BLOODY3D_MATRIXTRANSFORM
