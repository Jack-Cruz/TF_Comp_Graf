#version 330 core 

struct Material{
	sampler2D diffuse;
	sampler2D diffuse2;
	vec3 specular;
	float shininess;
};

struct Light{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 fragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3     viewPos;
uniform Material material;
uniform Light    light;
uniform float    grad;

out vec4 color;

void main() {
	vec3 auxTex = vec3(
					mix(texture(material.diffuse, TexCoords), 
						texture(material.diffuse2, TexCoords), 0.5+grad));

	// ambient
	
	vec3 ambient = light.ambient * auxTex;

	// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * auxTex;

	// specular
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); 
	vec3 specular =  light.specular * spec * material.specular;

 	// total
	vec3 result = ambient + diffuse + specular;
	color = vec4(result, 1.0f);
}