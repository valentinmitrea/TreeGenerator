#version 330

layout(location = 0) out vec4 out_color;

uniform vec3 light_position;
uniform vec3 camera_position;
uniform float far_plane;
uniform int element_to_draw;

uniform sampler2D ground_texture;
uniform samplerCube depth_cubemap;

in vec3 world_position;
in vec3 world_normal;
in vec2 texcoord;


bool isFragmentInShadow(vec3 N, vec3 L)
{
	vec3 position = world_position - light_position;
	float closest_depth = texture(depth_cubemap, position).x;
	float current_depth = length(position);

	//scalare closest_depth din intervalul [0;1] in valoarea originala
	closest_depth *= far_plane;

	//folosim bias pentru a nu intampina problema shadow acne
	float bias = max(0.5 * (1.0 - dot(N, L)), 0.05);

	return (current_depth - bias) > closest_depth;
}


void main()
{
	vec3 color, light;

	//daca suntem la desenarea ground-ului aplicam textura
	if (element_to_draw == 0)
		color = texture(ground_texture, texcoord).xyz;
	//altfel vom da copacului culoarea maro
	else
		color = vec3(0.62, 0.32, 0.17);

	//apoi, vom calcula lumina acelui fragment folosind modelul Blinn-Phong
	vec3 light_color = vec3(1.0, 1.0, 1.0);
	vec3 L = normalize(light_position - world_position);
	vec3 V = normalize(camera_position - world_position);
	vec3 N = normalize(world_normal);
	vec3 H = normalize(L + V);

	vec3 ambiental = vec3(0.15, 0.15, 0.15);
	vec3 diffuse = isFragmentInShadow(N, L) ? vec3(0.0, 0.0, 0.0) : max(dot(N, L), 0) * light_color;
	vec3 specular = isFragmentInShadow(N, L) ? vec3(0.0, 0.0, 0.0) : pow(max(dot(N, H), 0), 64) * light_color;

	//lumina este compusa din valoarea luminii ambientale + a luminii difuze + a luminii speculare
	light = ambiental + diffuse + specular;

	//culoarea finala este formata din culoarea calculata pentru fragment * lumina
	out_color = vec4(color * light, 1);
}
