out vec2 vf_TexCoord;
out vec2 vf_ViewspacePos;

void doFullscreenTriangle() {
	vf_TexCoord = vec2((gl_VertexID & 1) << 1, 1 - (gl_VertexID & 2));
	gl_Position = vec4(vf_TexCoord * 2 - 1, 0, 1);
	vf_ViewspacePos = gl_Position.xy / vec2(u.projection_mat[0][0], u.projection_mat[1][1]);
}
