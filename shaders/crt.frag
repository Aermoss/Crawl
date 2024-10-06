#version 330

#define PI 3.1415926538f

in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 size;
uniform vec2 curvature = vec2(3.0f);
uniform vec2 scanLineOpacity = vec2(1.0f);
uniform float vignetteOpacity = 1.0f;
uniform float vignetteRoundness = 1.0f;
uniform float brightness = 1.25f;

vec2 curveRemapUV(vec2 uv) {
    uv = uv * 2.0f - 1.0f;
    vec2 offset = abs(uv.yx) / vec2(curvature.x, curvature.y);
    uv = uv + uv * offset * offset;
    uv = uv * 0.5f + 0.5f;
    return uv;
}

vec4 scanLineIntensity(float uv, float resolution, float opacity) {
    float intensity = sin(uv * resolution * PI * 2.0f);
    intensity = ((0.5f * intensity) + 0.5f) * 0.9f + 0.1f;
    return vec4(vec3(pow(intensity, opacity)), 1.0f);
}

vec4 vignetteIntensity(vec2 uv, vec2 resolution, float opacity, float roundness) {
    float intensity = uv.x * uv.y * (1.0f - uv.x) * (1.0f - uv.y);
    return vec4(vec3(clamp(pow((resolution.x / roundness) * intensity, opacity), 0.0f, 1.0f)), 1.0f);
}

void main(void) {
    vec2 remappedUV = curveRemapUV(vec2(fragTexCoord.x, fragTexCoord.y));
    vec4 baseColor = texture2D(texture0, remappedUV);

    baseColor *= vignetteIntensity(remappedUV, size, vignetteOpacity, vignetteRoundness);
    baseColor *= scanLineIntensity(remappedUV.x, size.y, scanLineOpacity.x);
    baseColor *= scanLineIntensity(remappedUV.y, size.x, scanLineOpacity.y);
    baseColor *= vec4(vec3(brightness), 1.0f);

    if (remappedUV.x < 0.0f || remappedUV.y < 0.0f || remappedUV.x > 1.0f || remappedUV.y > 1.0f){
        fragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    } else {
        fragColor = baseColor;
    }
}