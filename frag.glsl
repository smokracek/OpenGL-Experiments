#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform float iTime;
uniform vec3 lightDirection;  
uniform vec3 baseColor;

uniform vec3 viewPos;

const float PI = 3.14159;

void main() {
	vec3 lightColor = vec3(1.0, 1.0, 1.0);

	float ambientStrength = 0.5;
	vec3 ambient = ambientStrength * lightColor;

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(-lightDirection);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// blinn-phong
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfwayDir = normalize(-lightDirection + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 128.0);
	vec3 specular = spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * baseColor;
	FragColor = vec4(result, 1.0);
}