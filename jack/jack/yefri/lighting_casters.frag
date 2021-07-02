#version 330 core 
out vec4 color;

struct Material{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct DirLight{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

struct SpotLight{
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

#define NR_POINT_LIGHTS 4

in vec3 fragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3     viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;


// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir); 
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {

	// properties
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - fragPos);


	// phase 1: directional lighting
	vec3 result = CalcDirLight(dirLight, norm, viewDir);

	// phase 2: point lights
	for(int i = 0; i < NR_POINT_LIGHTS; i++)
		result += CalcPointLight(pointLights[i], norm, fragPos, viewDir);
		

	// phase 3: Spot light
	result += CalcSpotLight(spotLight, norm, fragPos, viewDir);

	color = vec4(result, 1.0f);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){

	vec3 lightDir = normalize(-light.direction);

	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0),
	                 material.shininess);
	
	// combine results
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
	
	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){

	vec3 lightDir = normalize(light.position - fragPos);

	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);

	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	
	// attenuation
	float distance = length(light.position - fragPos);
	float attenuation =  1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// combine the results
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

	return (ambient + diffuse + specular);

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;


}


vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){

	vec3 lightDir = normalize(light.position - fragPos);

	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);

	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	// attenuation
	float distance = length(light.position - fragPos);
	float attenuation =  1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));


	// spotlight (soft edges)
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	// combine the results
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
	vec3 specular = light.specular *  spec * vec3(texture(material.specular, TexCoords));

	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;
	
	return (ambient + diffuse + specular);
}