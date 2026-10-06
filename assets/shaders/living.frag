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
    vec4 base = texture(source, uv);
    float t = sceneTime;
    // Colour and height masks keep masonry, the moon and mountains still.
    float blue = smoothstep(0.025, 0.13, base.b - base.r);
    float sea = smoothstep(0.205, 0.27, uv.y) * blue;
    float sky = (1.0 - smoothstep(0.075, 0.145, uv.y))
        * (1.0 - smoothstep(0.65, 0.88, base.r));
    float leaves = smoothstep(0.015, 0.065, base.g - max(base.r, base.b))
        * smoothstep(0.15, 0.25, uv.y) * (1.0 - smoothstep(0.55, 0.65, uv.y));
    vec2 shift = vec2(
        sin(uv.y * 210.0 - t * 1.1) + 0.4 * sin(uv.y * 370.0 + t * 0.7),
        0.35 * sin(uv.x * 160.0 + uv.y * 80.0 - t * 0.9));
    uv += shift * sea * mix(0.00025, 0.0012, uv.y);
    uv.x += sky * (0.0018 * sin(t * 0.065 + uv.y * 14.0)
        + 0.0008 * sin(t * 0.11 + uv.x * 11.0));
    uv.x += leaves * 0.00065 * sin(t * 0.85 + uv.x * 75.0 + uv.y * 40.0);
    vec4 c = texture(source, clamp(uv, vec2(0.001), vec2(0.999)));
    float windows = smoothstep(0.26, 0.65, base.r)
        * smoothstep(0.12, 0.4, base.r - base.b)
        * smoothstep(0.23, 0.28, uv.y);
    c.rgb *= 1.0 + sea * 0.025 * sin(uv.y * 310.0 - t * 1.3)
        + windows * darkness * 0.065 * sin(t * 0.7 + uv.x * 125.0 + uv.y * 97.0);
    fragColor = c * qt_Opacity;
}
