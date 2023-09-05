#version 330 core

layout(location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 ProjectionViewMatrix;

void main()
{
	TexCoords = aPos;
	vec4 pos = ProjectionViewMatrix * vec4(aPos * 1000.0, 1.0);
	gl_Position = pos.xyww;
}