#version 330

layout(location = 0) in vec2 position;

smooth out vec4 interpColor;

void main()
{
	gl_Position = vec4(position, 0.0, 1.0);
	interpColor = vec4(1.0, 0.0, 0.0, 1.0);
}
