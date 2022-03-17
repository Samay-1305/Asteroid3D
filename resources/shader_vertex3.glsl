#version 330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform float alpha;
out vec4 vertex_col;
out vec3 vertNoxA;
out vec3 fragPos;
out vec2 vertex_tex;

void main()
{
    vertex_col = vec4(1, 1, 1, alpha);
	gl_Position = P * V * M * vec4(vertPos, 1.0);
    fragPos = vec3(M * vec4(vertPos, 1.0));
    vertNoxA = vertNor;
    vertex_tex = vertTex;
}
