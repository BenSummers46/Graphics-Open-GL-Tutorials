#version 330 core

uniform sampler2D diffuseTex;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void){
    fragColour = texture(diffuseTex, IN.texCoord);
    //fragColour.a = 0.8;
    //fragColour = vec4(1.0, 0.0, 0.0, 1.0);
}