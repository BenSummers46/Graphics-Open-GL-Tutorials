#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D forestTex;
uniform sampler2D forestBump;
uniform sampler2D coastTex;
uniform sampler2D coastBump;
uniform sampler2D snowTex;
uniform sampler2D snowBump;

uniform vec3 cameraPos;
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform vec4 specularColour;
uniform float lightRadius;

in Vertex {
    vec3 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 tangent;
    vec3 binormal;
    vec3 worldPos;
    float height;
} IN;

out vec4 fragColour;

void main(void){
    vec3 incident = normalize(lightPos - IN.worldPos);
    vec3 viewDir = normalize(cameraPos - IN.worldPos);
    vec3 halfDir = normalize(incident + viewDir);

    mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

    vec4 diffuse;
    vec4 diffuse2;
    vec3 bumpNormal;
    float specFactor;
    float a;
    
    if(IN.height < 0.05){
        if(IN.height > 0.03){
            diffuse = texture(coastTex, IN.texCoord);
            diffuse2 = texture(forestTex, IN.texCoord);
            float difference = IN.height - 0.03;
            a = (difference / 0.02);
            bumpNormal = mix(texture(coastBump, IN.texCoord).rgb, texture(forestBump, IN.texCoord).rgb, a);
        }else{
            diffuse = texture(coastTex, IN.texCoord);
            diffuse2 = diffuse;
            a = 0;
            bumpNormal = texture(coastBump, IN.texCoord).rgb;
        }
        bumpNormal = normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));
        specFactor = clamp(dot(halfDir, bumpNormal), 0.0, 1.0);
        specFactor = pow(specFactor, 60.0);
    }else if(IN.height < 0.1){
        if(IN.height > 0.055){
            diffuse = texture(forestTex, IN.texCoord);
            diffuse2 = texture(diffuseTex, IN.texCoord);
            float difference = IN.height - 0.055;
            a = (difference / 0.045);
            bumpNormal = mix(texture(forestBump, IN.texCoord).rgb, texture(bumpTex, IN.texCoord).rgb, a);
        }else{
            diffuse = texture(forestTex, IN.texCoord);
            diffuse2 = diffuse;
            a = 0;
            bumpNormal = texture(forestBump, IN.texCoord).rgb;
        }
        bumpNormal = normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));
        specFactor = clamp(dot(halfDir, bumpNormal), 0.0, 1.0);
        specFactor = pow(specFactor, 70.0);
    }else if(IN.height < 0.4){
        if(IN.height > 0.30){
            diffuse = texture(diffuseTex, IN.texCoord);
            diffuse2 = texture(snowTex, IN.texCoord);
            float difference = IN.height - 0.30;
            a = (difference / 0.1);
            bumpNormal = mix(texture(bumpTex, IN.texCoord).rgb, texture(snowBump, IN.texCoord).rgb, a);
        }else{
            diffuse = texture(diffuseTex, IN.texCoord);
            diffuse2 = diffuse;
            a = 0;
            bumpNormal = texture(bumpTex, IN.texCoord).rgb;
        }
        bumpNormal = normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));
        specFactor = clamp(dot(halfDir, bumpNormal), 0.0, 1.0);
        specFactor = pow(specFactor, 30.0);
    }
    else{
        diffuse = texture(snowTex, IN.texCoord);
        diffuse2 = texture(snowTex, IN.texCoord);
        a = 1;
        bumpNormal = texture(snowBump, IN.texCoord).rgb;
        bumpNormal = normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));
        specFactor = clamp(dot(halfDir, bumpNormal), 0.0, 1.0);
        specFactor = pow(specFactor, 30.0);
    }

    float lambert = max(dot(incident, bumpNormal), 0.0f); 
    float distance = length(lightPos - IN.worldPos);
    float attenuation = 1.0f - clamp(distance / lightRadius, 0.0, 1.0);

    vec3 surface = (mix(diffuse.rgb, diffuse2.rgb, a) * lightColour.rgb);
    fragColour.rgb = surface * lambert * attenuation;
    fragColour.rgb += (specularColour.rgb * specFactor) * attenuation * 0.33;
    fragColour.rgb += surface * 0.1f;
    fragColour.a = diffuse.a;
}