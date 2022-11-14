#version 330 core

uniform samplerCube cubeTex;
uniform sampler2D waterBump;
uniform vec3 cameraPos;
//uniform vec4 lightColour;
//uniform vec3 lightPos;
//uniform vec4 specularColour;
//uniform float lightRadius;

out vec4 fragColour;

in Vertex {
    vec3 worldPos;
    vec3 normal;
    //vec2 tangent;
    //vec3 binormal;
    //vec2 texCoord;
} IN;

void main(void){
    //vec3 incident = normalize(lightPos - IN.worldPos);
    vec3 viewDir = normalize(cameraPos - IN.worldPos);
    //vec3 halfDir = normalize(incident + viewDir);

    //mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
    
    //vec4 bumpNormal = texture(waterBump, IN.texCoord);
    //bumpNormal = normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));
    //float specularFactor = clamp(dot(halfDir, bumpNormal), 0.0, 1.0);
    //specFactor = pow(specFactor, 70.0);

    vec3 reflectDir = reflect(-viewDir, normalize(IN.normal));
    vec4 reflectTex = texture(cubeTex, reflectDir);

    //float lambert = max(dot(incident, bumpNormal), 0.0f);
    //float distance = length(lightPos - IN.worldPos);
    //float attenuation = 1.0f - clamp(distance / lightRadius, 0.0, 1.0);

    //vec3 surface = reflectTex.rgb * lightColour.rgb;

    fragColour = (vec4(0.0, 0.0, 1.0, 0.25) * 0.5) + (reflectTex * 0.55);
}