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
layout(binding = 2) uniform sampler2D maskSource;
layout(binding = 3) uniform sampler2D nightSource;
vec4 nightSample(vec2 uv) {
    // Native 4K edge registration: +2 px at the residence, +5 at the
    // lighthouse, +3 at the church and +4 at the harbor. Originals stay intact.
    vec2 offset = vec2(mix(2.0, 5.0, smoothstep(.30, .87, uv.x)),
                       -1.0 + smoothstep(.20, .35, uv.y));
    return texture(nightSource, clamp(uv + offset / vec2(3840.0, 2160.0),
                                    vec2(0.0), vec2(1.0)));
}
// One surface and displacement field for both lighting modes. At the endpoints
// only one panorama is sampled; a blend needs both textures for four seconds.
vec4 artSample(vec2 uv) {
    if (darkness<=0.001) return texture(source,uv);
    if (darkness>=0.999) return nightSample(uv);
    return mix(texture(source,uv),nightSample(uv),darkness);
}

float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1,311.7)))*43758.5453); }
float noise(vec2 p) {
    vec2 i=floor(p), f=fract(p); f=f*f*(3.0-2.0*f);
    return mix(mix(hash(i),hash(i+vec2(1,0)),f.x),
               mix(hash(i+vec2(0,1)),hash(i+vec2(1,1)),f.x),f.y);
}
float wind(float t) { return .58*sin(t*.173)+.27*sin(t*.317+1.2)+.15*sin(t*.071+2.4); }
void main() {
    vec2 uv=qt_TexCoord0;
    float t=sceneTime;
    vec3 mask=texture(maskSource,uv).rgb;
    vec4 base=artSample(uv);
    float depth=smoothstep(.18,1.0,uv.y);
    // Quiet sheltered turquoise coves, stronger open ocean. Frequencies and
    // directions differ, and slow noise breaks the appearance of uniform bands.
    float shelter=1.0-.45*exp(-pow((uv.x-.49)/.20,2.0)-pow((uv.y-.64)/.13,2.0));
    float drift=noise(uv*vec2(12,9)+vec2(t*.023,-t*.017));
    float wide=sin(dot(uv,vec2(23,51))-t*.37+drift*1.2);
    float small=sin(dot(uv,vec2(99,174))+t*.83+drift*2.1);
    float ripple=sin(dot(uv,vec2(281,-327))-t*1.137);
    vec2 flow=vec2(wide+.31*small+.09*ripple,
                  .32*sin(dot(uv,vec2(51,-35))+t*.419)+.12*small);
    vec2 waterUV=uv+flow*mix(.00006,.00085,depth*depth)*shelter*mask.r;
    // Even the displaced sample must be water: no borrowing pixels from piers.
    float safe=mask.r*texture(maskSource,waterUV).r;
    vec4 c=mix(base,artSample(waterUV),safe);
    // Attached crowns bend by less than a pixel, trunks remain untouched.
    float crownPhase=dot(floor(uv*vec2(42,32)),vec2(2.41,1.73));
    float crown=mask.g*smoothstep(.16,.18,uv.y);
    vec2 leafUV=uv+vec2(.00037*(wind(t)+.24*sin(t*.91+crownPhase)),
                       .00013*sin(t*.713+crownPhase))*crown;
    c=mix(c,artSample(leafUV),crown*texture(maskSource,leafUV).g);
    // Sky and stars stay fixed: G contains only traced palm crowns.
    // Moving, irregular glints are surface light, rather than a screen flash.
    float normals=.58*wide+.28*small+.14*ripple;
    c.rgb*=1.0+safe*depth*normals*mix(.025,.015,darkness);
    float glint=pow(max(0.0,small*.65+ripple*.35),9.0)*noise(uv*vec2(360,190)+vec2(t*.16,t*.07));
    float moonPath=0.25; // Diffuse moonlight; the artwork has no visible moon.
    c.rgb+=safe*depth*glint*mix(vec3(.023,.033,.031),vec3(.033,.05,.072)*moonPath,darkness);
    // Warm reflected fragments remain attached to the painted harbor lights.
    float warm=smoothstep(.06,.26,base.r-base.b)*smoothstep(.53,.72,uv.y)*safe;
    c.rgb+=warm*darkness*vec3(.045,.024,.005)*(.5+.5*small);
    float shallows=safe*smoothstep(.08,.25,base.g-base.r);
    c.rgb+=shallows*(1.0-darkness*.8)*vec3(.013,.027,.021)*pow(max(0.,small*.5+ripple*.5),5.);
    // Almost all lamps are steady. Sparse bright cores vary independently.
    float seed=hash(floor(uv*vec2(150,120)));
    float lamps=smoothstep(.81,.99,base.r)*smoothstep(.26,.5,base.r-base.b)
        *step(.91,seed)*smoothstep(.3,.35,uv.y)*(1.0-smoothstep(.65,.7,uv.y));
    c.rgb*=1.0+lamps*darkness*.025*sin(t*(.19+seed*.17)+seed*53.);
    // Sparse bright star cores vary subtly without moving the sky.
    float stars=(1.0-smoothstep(.09,.16,uv.y))*smoothstep(.42,.7,base.b)*smoothstep(.11,.2,base.b-base.r)*step(.82,seed);
    c.rgb*=1.0+stars*darkness*.05*sin(t*(.13+seed*.23)+seed*61.);
    fragColor=c*qt_Opacity;
}
