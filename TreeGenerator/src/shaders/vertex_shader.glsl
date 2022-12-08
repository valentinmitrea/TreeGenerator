#version 330

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;	
layout(location = 2) in vec2 in_texcoord;

uniform mat4 model_matrix, view_matrix, projection_matrix;
uniform int element_to_draw;

out vec3 world_position;
out vec3 world_normal;
out vec2 texcoord;


void main()
{
	world_position = (model_matrix * vec4(in_position, 1)).xyz;
	world_normal = (model_matrix * vec4(in_normal, 1)).xyz;

	//vedem daca suntem la momentul de desenat ground
	if (element_to_draw == 0)
		texcoord = in_texcoord;

	gl_Position = projection_matrix * view_matrix * model_matrix * vec4(in_position, 1);
}
