#version 400 
 
smooth in vec4 vertPos;
smooth in vec4 vertNormal;
smooth in vec2 vertUV;

out vec4 outputColour; 

uniform vec4 camPos;

uniform vec4 kA;
uniform vec4 kD;
uniform vec4 kS;

//uniforms for texture data
uniform sampler2D DiffuseTexture;
uniform sampler2D SpecularTexture;
uniform sampler2D NormalTexture;

vec3 iA = vec3(0.25f, 0.25f, 0.25f);
vec3 iD = vec3(1.0f, 1.0f, 1.0f);
vec3 iS = vec3(1.0f, 1.0f, 1.0f);

vec4 lightDir = normalize(vec4(0.0f) - vec4(10.0f, 8.0f, 10.0f, 0.0f));

void main() 
{ 
	//get texture data from UV coords
	vec4 textureData = texture(NormalTexture, vertUV);
	vec3 Ambient = kA.xyz * iA; //ambient light

	//get lambertian time
	float nDl = max(0.0f, dot(normalize(vertNormal), -lightDir));
	vec3 Diffuse = kD.xyz * iD * nDl * textureData.rgb;

	vec3 R = reflect(lightDir, normalize(vertNormal)).xyz; //refracted light colour
	vec3 E = normalize(camPos - vertPos).xyz; //surface to eye vector

	float specTerm = pow(max(0.0f, dot(E, R)), kS.a); //specular term
	vec3 Specular = kS.xyz * iS * specTerm;

	outputColour = vec4(Ambient + Diffuse + Specular, 1.0f);
}