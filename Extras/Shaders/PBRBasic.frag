#version 460 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;

uniform vec3 uBaseColor;
uniform float uMetallic;
uniform float uRoughness;
uniform float uTransparency;
uniform vec3 uEmissionColor;
uniform float uEmissionStrength;
uniform float uSpecularStrength;

uniform vec3 uCameraPos;

const int MAX_LIGHTS = 16;
uniform int uLightCount;
uniform int uLightType[MAX_LIGHTS];
uniform vec3 uLightPos[MAX_LIGHTS];
uniform vec3 uLightDir[MAX_LIGHTS];
uniform vec3 uLightColor[MAX_LIGHTS];
uniform float uLightIntensity[MAX_LIGHTS];
uniform float uLightRadius[MAX_LIGHTS];
uniform float uLightInnerCone[MAX_LIGHTS];
uniform float uLightOuterCone[MAX_LIGHTS];
uniform vec2 uLightAreaSize[MAX_LIGHTS];

uniform vec3 uAmbientColor;
uniform float uAmbientIntensity;
uniform int uLightingMode;

uniform sampler2D uDiffuseMap;
uniform sampler2D uNormalMap;
uniform sampler2D uRoughnessMap;
uniform sampler2D uMetallicMap;
uniform sampler2D uAOMap;
uniform sampler2D uEmissiveMap;

uniform float uAlphaCutoff;

out vec4 FragColor;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / max(denom, 1e-6);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
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

    vec3 T = dp1 * duv2.y - dp2 * duv1.y;
    if (length(T) < 1e-6) {
        vec3 up = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
        T = cross(up, N);
    }
    T = normalize(T - N * dot(N, T));
    vec3 B = normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    vec3 nMap = texture(uNormalMap, vUV).rgb;
    vec3 N_tangent = normalize(nMap * 2.0 - 1.0);
    bool hasNormalMap = length(N_tangent - vec3(0.0,0.0,1.0)) > 0.001;
    if (hasNormalMap)
        N = normalize(TBN * N_tangent);

    vec4 diffuseSample = texture(uDiffuseMap, vUV);
    vec3 albedo = uBaseColor * diffuseSample.rgb;
    float diffuseAlpha = diffuseSample.a;

    float metallic = clamp(uMetallic * texture(uMetallicMap, vUV).r, 0.0, 1.0);
    float roughness = clamp(uRoughness * texture(uRoughnessMap, vUV).r, 0.05, 1.0);
    float ao = texture(uAOMap, vUV).r;

    float alpha = clamp(diffuseAlpha * (1.0 - uTransparency), 0.0, 1.0);

    if (alpha <= uAlphaCutoff)
        discard;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    if (uLightingMode != 0)
    {
        for (int i = 0; i < uLightCount && i < MAX_LIGHTS; ++i)
        {
            int t = uLightType[i];
            vec3 radiance = uLightColor[i] * uLightIntensity[i];

            vec3 Ldir;
            float attenuation = 1.0;

            if (t == 0)
            {
                Ldir = normalize(-uLightDir[i]);
            }
            else
            {
                vec3 toLight = uLightPos[i] - vWorldPos;
                float dist = length(toLight);
                Ldir = normalize(toLight);
                float r = max(uLightRadius[i], 1e-4);

                float invDist2 = 1.0 / max(dist * dist, 1e-6);
                float range = dist / r;
                float rangeAtten = 1.0 - smoothstep(0.9, 1.0, range);
                attenuation = invDist2 * rangeAtten;

                if (t == 2)
                {
                    vec3 spotDir = normalize(-uLightDir[i]);
                    float cosTheta = dot(spotDir, Ldir);
                    float inner = uLightInnerCone[i];
                    float outer = uLightOuterCone[i];
                    float spot = 0.0;
                    if (cosTheta > outer)
                        spot = clamp((cosTheta - outer) / max(inner - outer, 1e-4), 0.0, 1.0);
                    attenuation *= spot;
                }
                else if (t == 3)
                {
                    vec3 areaDir = normalize(-uLightDir[i]);
                    float area = max(1e-4, uLightAreaSize[i].x * uLightAreaSize[i].y);
                    float NdotArea = max(0.0, dot(N, areaDir));
                    attenuation = (area * NdotArea) / max(dist * dist + 1.0, 1e-6);
                    Ldir = areaDir;
                }
            }

            float NdotL = max(dot(N, Ldir), 0.0);
            if (NdotL <= 0.0) continue;

            vec3 H = normalize(V + Ldir);
            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, Ldir, roughness);
            vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 nominator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 1e-3) * max(NdotL, 1e-3);
            vec3 specular = nominator / denominator;

            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metallic;

            specular *= uSpecularStrength;

            Lo += (kD * albedo / PI * alpha + specular) * radiance * attenuation * NdotL;
        }
    }

    vec3 ambient = uAmbientColor * uAmbientIntensity * albedo * ao * alpha;

    vec3 emissive = uEmissionColor * uEmissionStrength * texture(uEmissiveMap, vUV).rgb * alpha;

    vec3 color = ambient + Lo + emissive;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, alpha);
}
