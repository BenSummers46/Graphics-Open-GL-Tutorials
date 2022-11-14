#version 330 core

uniform mat4 modelMatrix;
uniform mat4 projMatrix;
uniform mat4 viewMatrix;

in vec3 position;
in vec4 colour;
in vec3 normal;
in vec4 tangent;
in vec2 texCoord;

out Vertex {
    vec2 texCoord;
    vec4 colour;
    vec3 normal;
    vec3 tangent;
    vec3 binormal;
    vec3 worldPos;
} OUT;

void main(void){
    vec4 worldPos = (modelMatrix * vec4(position, 1));
    OUT.worldPos = worldPos.xyz;
    OUT.colour = colour;

    OUT.texCoord = texCoord;
    OUT.texCoord.y = 1 - OUT.texCoord.y;

    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

    vec3 wNormal = normalize(normalMatrix * normalize(normal));
    vec3 wTangent = normalize(normalMatrix * normalize(tangent.xyz));

    OUT.normal = wNormal;
    OUT.tangent = wTangent;
    OUT.binormal = cross(wTangent, wNormal) * tangent.w;

    gl_Position = (projMatrix * viewMatrix) * worldPos;
}