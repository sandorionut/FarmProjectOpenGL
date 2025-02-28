#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;

out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;
uniform vec3 lightPos;
uniform mat4 view;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.8f;
float shininess = 32.0f;
float constant = 1.0f;
float linear = 0.7f;
float quadratic = 1.8f;

in vec4 fragPosLightSpace;

void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

	// compute half vector
	vec3 halfDir = normalize(lightDirN + viewDirN);
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

void computePointLightComponents() {

	//compute distance of light
	float dist = length(fPosEye.xyz);

	specularStrength = 0.8f;

	//compute attenuation
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin

	//transform normal
	vec3 normalEye = normalize(fNormal);

	//compute light direction
	vec4 lightPosEye = view * vec4(lightPos ,1.0f);
	vec3 lightDirN = normalize(lightPosEye.xyz - fPosEye.xyz);

	//compute view direction
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

	// compute half vector
	vec3 halfDir = normalize(lightDirN + viewDirN);

	//compute ambient light
	ambient += ambientStrength * lightColor * att;

	//compute diffuse light
	diffuse += max(dot(normalEye, lightDirN), 0.0f) * lightColor * att;

	//compute specular light
	vec3 reflection = reflect(-halfDir, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular += att * specularStrength * specCoeff * lightColor;
}

float computeShadow()
{
	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;

	// Check whether current frag pos is in shadow
	//float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

	// Check whether current frag pos is in shadow
	float bias = max(0.05f * (1.0f - dot(fNormal, lightDir)), 0.005f);
	float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;

	if(normalizedCoords.z > 1.0f)
		return 0.0f;

	return shadow;
}

void main()
{
	computeLightComponents();
	computePointLightComponents();

	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange

	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	//modulate with shadow
	float shadow = computeShadow();
	vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);

    fColor = vec4(color, 1.0f);
}