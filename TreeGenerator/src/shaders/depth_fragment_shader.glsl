#version 330

uniform vec3 light_position;
uniform float far_plane;

in vec3 world_position;


void main()
{
	//distanta intre fragment si pozitia luminii
	//aceasta distanta este transformata in intervalul [0;1] prin impartirea la far_plane
	float distance = length(world_position - light_position) / far_plane;

	//salvam adancimea ca fiind aceasta distanta
	gl_FragDepth = distance;
}
