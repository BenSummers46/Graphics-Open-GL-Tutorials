#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform float time;

uniform int wave_amount = 4;
uniform struct Gerstner{
    vec2 direction;
    float amplitude;
    float steepness;
    float frequency;
    float speed;
} gerstner[4] = Gerstner[4](
    Gerstner(vec2(1.0, 0.0), 15.0, 1.5, 0.015, 3.0),
    Gerstner(vec2(0.0, 1.0), 7.5, 0.5, 0.001, 0.5),
    Gerstner(vec2(0.5, 0.5), 22.5, 0.5, 0.01, 1.5),
    Gerstner(vec2(1.0, 1.0), 12.5, 1.0, 0.001, 2.0)
);


in vec3 position;
in vec3 normal;

out Vertex {
    vec3 worldPos;
    vec3 normal;
} OUT;

vec3 gerstner_normal(vec3 position, float time){
    vec3 wave_normal = vec3(0.0, 1.0, 0.0);
    for(int i = 0; i < wave_amount; ++i){
        float proj = dot(position.xz, gerstner[i].direction),
        phase = time * gerstner[i].speed,
        psi = proj * gerstner[i].frequency + phase,
        af = gerstner[i].amplitude * gerstner[i].frequency,
        alpha = af * sin(psi);

        wave_normal.y -= gerstner[i].steepness * alpha;

        float x = gerstner[i].direction.x,
        y = gerstner[i].direction.y,
        omega = af * cos(psi);

        wave_normal.x -= x * omega;
        wave_normal.z -= y * omega;
    }
    return wave_normal;
}

vec3 gerstner_wave_pos(vec2 position, float time){
    vec3 wave_position = vec3(position.x, 0, position.y);
    for(int i = 0; i < wave_amount; ++i){
        float proj = dot(position, gerstner[i].direction),
        phase = time * gerstner[i].speed,
        theta = proj * gerstner[i].frequency + phase,
        height = gerstner[i].amplitude * sin(theta);

        wave_position.y += height;

        float maximum_width = gerstner[i].steepness * gerstner[i].amplitude,
        width = maximum_width * cos(theta),
        x = gerstner[i].direction.x,
        y = gerstner[i].direction.y;

        wave_position.x += x * width;
        wave_position.z += y * width;
    }
    return wave_position;
}

vec3 gerstner_wave(vec2 position, float time, inout vec3 normal){
    vec3 wave_pos = gerstner_wave_pos(position, time);
    normal = gerstner_normal(wave_pos, time);
    return wave_pos;
}

void main(void){
    vec3 newNorm = normal;
    vec3 vertPosition = gerstner_wave(position.xz, time, newNorm);
    
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    OUT.normal = normalize(normalMatrix * normalize(newNorm));
    
    vec4 worldPos = (modelMatrix * vec4(vertPosition, 1));
    OUT.worldPos = worldPos.xyz;

    gl_Position = (projMatrix * viewMatrix) * worldPos;
}