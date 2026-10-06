#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float sceneTime;
    float darkness;
};
layout(binding = 1) uniform sampler2D source;

void main() {
    vec2 uv = qt_TexCoord0;
    float t = sceneTime;
    // Cloth billows between its fastenings; the mast and wooden hull stay rigid.
    float cloth = smoothstep(0.08, 0.22, uv.y) * (1.0 - smoothstep(0.57, 0.72, uv.y));
    float mastDistance = smoothstep(0.015, 0.12, abs(uv.x - (0.50 + 0.07 * uv.y)));
    float breath = sin(t * 1.15 + uv.y * 9.0) + 0.32 * sin(t * 2.05 - uv.y * 17.0);
    uv.x += 0.0045 * cloth * mastDistance * breath;
    vec4 c = texture(source, clamp(uv, vec2(0.001), vec2(0.999)));
    float canvas = smoothstep(0.26, 0.6, c.g) * cloth;
    float lantern = smoothstep(0.82, 0.98, c.r) * smoothstep(0.45, 0.75, c.g) * smoothstep(0.2, 0.55, c.r - c.b)
        * smoothstep(0.60, 0.69, uv.y);
    float flame = 0.65 * sin(t * 2.1 + uv.x * 43.0) + 0.35 * sin(t * 3.7 + uv.x * 79.0);
    c.rgb *= 1.0 + canvas * 0.045 * breath + lantern * darkness * 0.10 * flame;
    // Preserve premultiplied alpha along rigging and sail edges.
    c.rgb = min(c.rgb, vec3(c.a));
    fragColor = c * qt_Opacity;
}
