#version 330 core

uniform sampler2D diffuseTex;

in Vertex{
    vec2 texCoord;
    vec4 weights;
} IN;

out vec4 fragColour;

void main(void){
    fragColour = texture(diffuseTex, IN.texCoord);
}