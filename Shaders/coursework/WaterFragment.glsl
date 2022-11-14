#version 330 core

uniform samplerCube cubeTex;
uniform vec3 cameraPos;

out vec4 fragColour;

in Vertex {
    vec3 worldPos;
    vec3 normal;
    float height;
    
} IN;

void main(void){
    
    vec3 viewDir = normalize(cameraPos - IN.worldPos);

    vec3 reflectDir = reflect(-viewDir, normalize(IN.normal));
    vec4 reflectTex = texture(cubeTex, reflectDir);

    if(IN.height > 30.0){
        fragColour = (vec4(1.0, 1.0, 1.0, 0.15) * 0.5) + (reflectTex * 0.55);
    }else{
        fragColour = (vec4(0.0, 0.0, 1.0, 0.25) * 0.5) + (reflectTex * 0.55);
    }
}