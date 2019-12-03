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
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

in vec2 fragTexCoords;
in vec4 fragPosLightSpace;

float computeShadow()
{
	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	
	if (normalizedCoords.z > 1.0f)
		return 0.0f;
	
	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
//o = vec3(closestDepth);	
	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;
	
	// Check whether current frag pos is in shadow
	float bias = 0.005;
	float shadow = currentDepth -bias > closestDepth ? 1.0 : 0.0;
	
	//return 0.0f;
	return shadow;

}

vec3 computeLightComponents(vec3 lightColor)
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
	
	float shadow = computeShadow();
	
	vec3 color = min((ambient + (1.0f -shadow)*diffuse) + (1.0f -shadow)*specular, 1.0f);
	
	return color;
}

void main() 
{
	//computeLightComponents();
	
	vec3 color1 = computeLightComponents(lightColor);
    
    fColor = vec4(color1, 0.0f);
	//fColor = texture(shadowMap, fragTexCoords);
}
