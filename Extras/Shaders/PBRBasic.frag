#version 460 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;

uniform vec3  uBaseColor;
uniform float uMetallic;
uniform float uRoughness;
uniform float uTransparency;
uniform vec3  uEmissionColor;
uniform float uEmissionStrength;
uniform float uSpecularStrength;
uniform float uAlphaCutoff;

uniform bool uHasDiffuseMap;
uniform bool uHasNormalMap;
uniform bool uHasRoughnessMap;
uniform bool uHasMetallicMap;
uniform bool uHasAOMap;
uniform bool uHasEmissiveMap;

uniform sampler2D uDiffuseMap;
uniform sampler2D uNormalMap;
uniform sampler2D uRoughnessMap;
uniform sampler2D uMetallicMap;
uniform sampler2D uAOMap;
uniform sampler2D uEmissiveMap;

uniform vec3 uCameraPos;

const int MAX_LIGHTS = 16;

uniform int   uLightCount;
uniform int   uLightType[MAX_LIGHTS];
uniform vec3  uLightPos[MAX_LIGHTS];
uniform vec3  uLightDir[MAX_LIGHTS];
uniform vec3  uLightColor[MAX_LIGHTS];
uniform float uLightIntensity[MAX_LIGHTS];
uniform float uLightRadius[MAX_LIGHTS];
uniform float uLightInnerCone[MAX_LIGHTS];
uniform float uLightOuterCone[MAX_LIGHTS];
uniform vec2  uLightAreaSize[MAX_LIGHTS];

uniform vec3  uAmbientColor;
uniform float uAmbientIntensity;
uniform int   uLightingMode;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    return a2 / max(PI * denom * denom, 1e-6);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / max(NdotV * (1.0 - k) + k, 1e-6);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vWorldPos);

    vec3 dp1 = dFdx(vWorldPos);
    vec3 dp2 = dFdy(vWorldPos);
    vec2 duv1 = dFdx(vUV);
    vec2 duv2 = dFdy(vUV);

    float det = duv1.x * duv2.y - duv2.x * duv1.y;
    vec3 T;

    if (abs(det) < 1e-6)
    {
        vec3 up = abs(N.y) < 0.999 ? vec3(0,1,0) : vec3(1,0,0);
        T = normalize(cross(up, N));
    }
    else
    {
        T = normalize((dp1 * duv2.y - dp2 * duv1.y) / det);
    }

    T = normalize(T - N * dot(N, T));
    vec3 B = normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    if (uHasNormalMap)
    {
        vec3 nMap = texture(uNormalMap, vUV).rgb;

        if (length(nMap) > 0.01)
        {
            vec3 Nt = normalize(nMap * 2.0 - 1.0);
            N = normalize(TBN * Nt);
        }
    }

    vec4 diffuseSample = uHasDiffuseMap
        ? texture(uDiffuseMap, vUV)
        : vec4(1.0);

    vec3 albedo = uBaseColor * diffuseSample.rgb;
    float alpha = clamp(diffuseSample.a * (1.0 - uTransparency), 0.0, 1.0);

    if (uHasDiffuseMap && alpha <= uAlphaCutoff)
        discard;

    float metallic  = uMetallic;
    float roughness = clamp(uRoughness, 0.05, 1.0);
    float ao        = 1.0;

    if (uHasMetallicMap)
        metallic = clamp(metallic * texture(uMetallicMap, vUV).r, 0.0, 1.0);

    if (uHasRoughnessMap)
        roughness = clamp(roughness * texture(uRoughnessMap, vUV).r, 0.05, 1.0);

    if (uHasAOMap)
        ao = texture(uAOMap, vUV).r;

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);

    if (uLightingMode != 0)
    {
        for (int i = 0; i < min(uLightCount, MAX_LIGHTS); i++)
        {
            vec3 L;
            float attenuation = 1.0;
            vec3 radiance = uLightColor[i] * uLightIntensity[i];

            if (uLightType[i] == 0)
            {
                L = normalize(-uLightDir[i]);
            }
            else
            {
                vec3 toLight = uLightPos[i] - vWorldPos;
                float dist = length(toLight);
                L = normalize(toLight);

                attenuation = 1.0 / max(dist * dist, 1e-4);

                if (uLightType[i] == 2)
                {
                    float cosTheta = dot(normalize(-uLightDir[i]), L);
                    float spot = smoothstep(uLightOuterCone[i], uLightInnerCone[i], cosTheta);
                    attenuation *= spot;
                }
            }

            float NdotL = max(dot(N, L), 0.0);
            if (NdotL <= 0.0) continue;

            vec3 H = normalize(V + L);

            float NDF = DistributionGGX(N, H, roughness);
            float G   = GeometrySmith(N, V, L, roughness);
            vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 specular = (NDF * G * F) /
                            max(4.0 * max(dot(N,V),1e-3) * NdotL, 1e-6);

            specular *= uSpecularStrength;

            vec3 kS = F;
            vec3 kD = (1.0 - kS) * (1.0 - metallic);

            Lo += (kD * albedo / PI + specular) * radiance * attenuation * NdotL;
        }
    }

    vec3 ambient = uAmbientColor * uAmbientIntensity * albedo * ao;

    vec3 emissive = vec3(0.0);
    if (uHasEmissiveMap)
        emissive = uEmissionColor * uEmissionStrength *
                   texture(uEmissiveMap, vUV).rgb;

    vec3 color = ambient + Lo + emissive;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, alpha);
}
