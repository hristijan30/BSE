#version 460 core

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoord;

uniform sampler2D uAlbedo;
uniform sampler2D uNormal; // Optional
uniform sampler2D uRoughness; // Optional

uniform float uMetallic = 0.0;
uniform float uEmission = 0.0;
uniform float uOpacity  = 1.0;

out vec4 FragColor;

vec3 ApplyLighting(vec3 color, vec3 normal)
{
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normal, lightDir), 0.0);
    return color * (0.3 + 0.7 * diff);
}

void main()
{
    vec3 albedo = texture(uAlbedo, vTexCoord).rgb;

    vec3 normal = normalize(vNormal);
    #ifdef USE_NORMAL
        normal = texture(uNormal, vTexCoord).rgb * 2.0 - 1.0;
        normal = normalize(normal);
    #endif

    float roughness = 1.0;
    #ifdef USE_ROUGHNESS
        roughness = texture(uRoughness, vTexCoord).r;
    #endif

    albedo = mix(albedo, vec3(1.0), uMetallic);
    albedo += uEmission;

    vec3 color = ApplyLighting(albedo, normal);

    FragColor = vec4(color, uOpacity);
}
