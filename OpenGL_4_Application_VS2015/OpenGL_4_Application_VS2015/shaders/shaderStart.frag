#version 410 core

in vec3 normal;
in vec4 fragPosEye;

out vec4 fColor;

//lighting
uniform	mat3 normalMatrix;

uniform	vec3 lightDir;
uniform	vec3 lightColor;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float constant = 1.0f;
float linear = 0.045f;
float quadratic = 0.0075f;

uniform mat4 view;
uniform vec3 lightPos;

uniform vec3 lightPos2;

uniform vec3 lightColor2;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

in vec2 fragTexCoords;

vec3 computeLightComponentsDir(vec3 lightDir, vec3 lightColor)
{	

	//vec3 lightPosEye = vec3(view * vec4(lightPos,1.0)); 

	//compute distance to light
	//float dist = length(lightPosEye -fragPosEye.xyz);
	//compute attenuation
	//float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
	float att = 1.0f;
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir - fragPosEye.xyz);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
		
	//compute ambient light
	ambient = att * ambientStrength * lightColor;
	ambient *= vec3(texture(diffuseTexture,fragTexCoords));
	
	//compute diffuse light
	diffuse = att * max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	diffuse *= vec3(texture(diffuseTexture,fragTexCoords));
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
	
	//compute specular light
	//vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, halfVector), 0.0f), shininess);
	specular = att * specularStrength * specCoeff * lightColor;
	specular *= vec3(texture(specularTexture,fragTexCoords));
	
	vec3 color = min((ambient + diffuse) + specular, 1.0f);
	
	return color;
}

vec3 computeLightComponentsPoint(vec3 lightPos, vec3 lightColor)
{	

	vec3 lightPosEye = vec3(view * vec4(lightPos,1.0)); 

	//compute distance to light
	float dist = length(lightPosEye -fragPosEye.xyz);
	//compute attenuation
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
	//att = 1.0f;
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightPosEye - fragPosEye.xyz);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
		
	//compute ambient light
	ambient = att * ambientStrength * lightColor;
	ambient *= vec3(texture(diffuseTexture,fragTexCoords));
	
	//compute diffuse light
	diffuse = att * max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	diffuse *= vec3(texture(diffuseTexture,fragTexCoords));
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
	
	//compute specular light
	//vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, halfVector), 0.0f), shininess);
	specular = att * specularStrength * specCoeff * lightColor;
	specular *= vec3(texture(specularTexture,fragTexCoords));
	
	vec3 color = min((ambient + diffuse) + specular, 1.0f);
	
	return color;
}

void main() 
{
	//computeLightComponents();
	
	vec3 color1 = computeLightComponentsPoint(lightPos,lightColor);
	vec3 color2 = computeLightComponentsDir(lightDir,lightColor2);
    
    fColor = vec4(color1*2 + color2, 1.0f);
}
