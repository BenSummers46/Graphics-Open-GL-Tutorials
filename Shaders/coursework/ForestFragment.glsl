#version 330 core

uniform sampler2D diffuseTex;

in Vertex {
    vec2 texCoord;
    float height;
} IN;

out vec4 fragColour;

void main(void){
    fragColour = texture(diffuseTex, IN.texCoord);
    fragColour.a = 1 - (IN.height / 10); 
    
}