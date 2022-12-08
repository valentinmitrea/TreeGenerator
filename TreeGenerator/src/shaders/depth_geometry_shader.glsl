#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 light_view_matrices[6], light_projection_matrix;

out vec3 world_position;


void main()
{
	for (int face = 0; face < 6; face++)
	{
		//specificare catre ce fata din cubemap randam
		gl_Layer = face;

		for (int i = 0; i < gl_in.length(); i++)
		{
			world_position = gl_in[i].gl_Position.xyz;
			gl_Position = light_projection_matrix * light_view_matrices[face] * gl_in[i].gl_Position;
			EmitVertex();
		}

		EndPrimitive();
	}
}
