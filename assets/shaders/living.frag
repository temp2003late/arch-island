#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float sceneTime;
    float darkness;
    float heat;
    float lookX;
    float lookY;
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

float noise(vec2 p) {
    vec2 cell = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    vec2 k = vec2(127.1, 311.7);
    float a = fract(sin(dot(cell, k)) * 43758.5453);
    float b = fract(sin(dot(cell + vec2(1, 0), k)) * 43758.5453);
    float c = fract(sin(dot(cell + vec2(0, 1), k)) * 43758.5453);
    float d = fract(sin(dot(cell + vec2(1, 1), k)) * 43758.5453);
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
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
    // A relief map separates sky, distant sea, island and close waves.
    float land = (1.0 - smoothstep(0.01, 0.10, base.b - base.r))
        * smoothstep(0.26, 0.34, uv.y) * (1.0 - smoothstep(0.70, 0.79, uv.y))
        * smoothstep(0.19, 0.29, uv.x) * (1.0 - smoothstep(0.76, 0.85, uv.x));
    float depth = land * 0.65 + sea * smoothstep(0.28, 1.0, uv.y);
    uv += vec2(lookX * 0.0020, lookY * 0.0014) * depth;
    vec2 shift = vec2(
        sin(uv.y * 210.0 - t * 1.1) + 0.4 * sin(uv.y * 370.0 + t * 0.7),
        0.35 * sin(uv.x * 160.0 + uv.y * 80.0 - t * 0.9));
    uv += shift * sea * mix(0.00015, 0.0020, uv.y * uv.y);
    uv.x += sky * (0.0018 * sin(t * 0.065 + uv.y * 14.0)
        + 0.0008 * sin(t * 0.11 + uv.x * 11.0));
    float gust = 0.65 + 0.35 * sin(t * 0.24 + uv.y * 9.0);
    uv.x += leaves * 0.0011 * gust * sin(t * 1.15 + uv.x * 75.0 + uv.y * 40.0);
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
    // Wind carries large, soft cloud shadows across terraces and foliage.
    float cloud = noise(uv * vec2(7.0, 5.0) + vec2(t * 0.015, -t * 0.008));
    c.rgb *= 1.0 - land * (1.0 - darkness) * 0.17 * smoothstep(0.35, 0.8, cloud);

    // Crossing wave normals catch light at different depths; turquoise shallows
    // receive moving caustics while the horizon stays quiet and far away.
    float wave = sin(uv.y * 390.0 - t * 1.65 + sin(uv.x * 80.0 + t * 0.3))
               + 0.45 * sin(uv.x * 220.0 + uv.y * 290.0 + t * 1.2);
    float nearSea = sea * smoothstep(0.30, 0.95, uv.y);
    c.rgb *= 1.0 + nearSea * wave * mix(0.045, 0.025, darkness);
    float shallows = sea * smoothstep(0.08, 0.25, base.g - base.r)
        * (1.0 - smoothstep(0.1, 0.32, base.b - base.g));
    float caustic = pow(0.5 + 0.5 * sin(uv.x * 260.0 + sin(uv.y * 190.0 + t) * 2.0 - t * 0.8), 8.0);
    c.rgb += vec3(0.045, 0.08, 0.065) * caustic * shallows * (1.0 - darkness * 0.7);

    // The lava illuminates its own rock, with slow heat-driven convection.
    float crater = (1.0 - smoothstep(0.06, 0.15, distance(uv, vec2(0.43, 0.25))))
        * smoothstep(0.22, 0.55, base.r - base.g);
    c.rgb *= 1.0 + crater * (0.04 + heat * 0.12) * sin(t * 1.7 + uv.y * 65.0);

    // Atmospheric perspective softens the distant water, never the town.
    float horizon = exp(-pow((uv.y - 0.265) / 0.045, 2.0)) * sea;
    c.rgb = mix(c.rgb, mix(vec3(0.58, 0.74, 0.82), vec3(0.10, 0.18, 0.28), darkness),
                horizon * mix(0.12, 0.045, darkness));
    float stars = (1.0 - smoothstep(0.04, 0.13, uv.y))
        * smoothstep(0.32, 0.75, base.b) * (1.0 - smoothstep(0.62, 0.85, base.r));
    c.rgb *= 1.0 + stars * darkness * 0.22 * lightRhythm(uv * 3.0, t * 0.35);
    fragColor = c * qt_Opacity;
}
