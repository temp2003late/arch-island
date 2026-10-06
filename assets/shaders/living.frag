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

float lampPulse(vec2 cell, float t) {
    float seed = fract(sin(dot(cell, vec2(127.1, 311.7))) * 43758.5453);
    return 0.65 * sin(t * (0.7 + seed * 0.5) + seed * 31.0)
         + 0.35 * sin(t * (1.8 + seed * 0.7) + seed * 57.0);
}

float lightRhythm(vec2 uv, float t) {
    // Independent, smoothly blended local rhythms; no travelling brightness bands.
    vec2 p = uv * vec2(100.0, 80.0);
    vec2 cell = floor(p);
    vec2 f = smoothstep(vec2(0.0), vec2(1.0), fract(p));
    return mix(mix(lampPulse(cell, t), lampPulse(cell + vec2(1, 0), t), f.x),
               mix(lampPulse(cell + vec2(0, 1), t), lampPulse(cell + vec2(1, 1), t), f.x), f.y);
}

void main() {
    vec2 uv = qt_TexCoord0;
    vec4 base = texture(source, uv);
    float t = sceneTime;
    // Colour and height masks keep masonry, the moon and mountains still.
    float blue = smoothstep(0.025, 0.13, base.b - base.r);
    float warm = smoothstep(0.12, 0.38, base.r - base.b);
    float foregroundWater = smoothstep(0.72, 0.80, uv.y);
    float sea = smoothstep(0.205, 0.27, uv.y) * max(blue, warm * foregroundWater);
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
    // Select bright, compact warm sources rather than illuminated walls or paving.
    vec3 nearby = vec3(0.0);
    const vec2 offsets[4] = vec2[4](vec2(-0.003, 0), vec2(0.003, 0),
                                    vec2(0, -0.005), vec2(0, 0.005));
    for (int i = 0; i < 4; ++i)
        nearby += texture(source, clamp(uv + offsets[i], vec2(0.001), vec2(0.999))).rgb * 0.25;
    float compactSource = smoothstep(0.025, 0.16, base.r - nearby.r);
    float lights = smoothstep(0.78, 0.98, base.r)
        * smoothstep(0.40, 0.72, base.g)
        * smoothstep(0.18, 0.45, base.r - base.b)
        * compactSource
        * smoothstep(0.17, 0.22, uv.y)
        * (1.0 - smoothstep(0.68, 0.73, uv.y));
    // Original light spill and reflections remain intact; only lamp/window cores vary.
    c.rgb *= 1.0 + lights * darkness * 0.10 * lightRhythm(uv, t);
    fragColor = c * qt_Opacity;
}
