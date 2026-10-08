#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float sceneTime;
    float darkness;
    float heat;
};
layout(binding = 1) uniform sampler2D source;
layout(binding = 2) uniform sampler2D maskSource;
layout(binding = 3) uniform sampler2D nightSource;
vec4 nightSample(vec2 uv) {
    // Measured local registration: the night tower/crane are one native pixel
    // left of daylight. Align before blending, with soft stationary boundaries.
    float tower=smoothstep(.675,.679,uv.x)*(1.-smoothstep(.698,.702,uv.x))
        *smoothstep(.242,.253,uv.y)*(1.-smoothstep(.39,.40,uv.y));
    float crane=smoothstep(.716,.721,uv.x)*(1.-smoothstep(.774,.779,uv.x))
        *smoothstep(.612,.626,uv.y)*(1.-smoothstep(.725,.74,uv.y));
    return texture(nightSource,uv-vec2(max(tower,crane)/1774.,0));
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
    float depth=smoothstep(.265,1.0,uv.y);
    // Quiet sheltered turquoise coves, stronger open ocean. Frequencies and
    // directions differ, and slow noise breaks the appearance of uniform bands.
    float shelter=1.0-.45*exp(-pow((uv.x-.31)/.13,2.0)-pow((uv.y-.59)/.13,2.0));
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
    // Cloud advection is restricted to upper sky. The moon and star cores are
    // stationary; only cloud pixels in the common panorama move imperceptibly.
    float sky=1.0-smoothstep(.115,.165,uv.y);
    float moon=1.0-smoothstep(.026,.043,length((uv-vec2(.770,.123))*vec2(1,0.5)));
    float cloud=sky*(1.0-moon)*mask.g;
    vec2 cloudUV=uv+vec2(.00035*sin(t*.019+uv.y*9),.00004*sin(t*.027))*cloud;
    c=mix(c,artSample(cloudUV),cloud*.45);
    // Moving, irregular glints are surface light, rather than a screen flash.
    float normals=.58*wide+.28*small+.14*ripple;
    c.rgb*=1.0+safe*depth*normals*mix(.025,.015,darkness);
    float glint=pow(max(0.0,small*.65+ripple*.35),9.0)*noise(uv*vec2(360,190)+vec2(t*.16,t*.07));
    float moonPath=exp(-pow((uv.x-.77)/(.014+.085*depth),2.0))*smoothstep(.255,.34,uv.y);
    c.rgb+=safe*depth*glint*mix(vec3(.023,.033,.031),vec3(.033,.05,.072)*moonPath,darkness);
    // Warm reflected fragments remain attached to the painted harbor lights.
    float warm=smoothstep(.06,.26,base.r-base.b)*smoothstep(.53,.72,uv.y)*safe;
    c.rgb+=warm*darkness*vec3(.045,.024,.005)*(.5+.5*small);
    float shallows=safe*smoothstep(.08,.25,base.g-base.r);
    c.rgb+=shallows*(1.0-darkness*.8)*vec3(.013,.027,.021)*pow(max(0.,small*.5+ripple*.5),5.);
    // Existing hot channels only. Flow travels downhill, with no new lava paths.
    float lava=mask.b;
    vec2 hotUV=uv+vec2(.000045*sin(t*.617+uv.y*83.),.000025*sin(t*.431+uv.x*71.))*lava;
    c=mix(c,artSample(hotUV),lava*texture(maskSource,hotUV).b*.25);
    float hot=.65+.22*sin(uv.y*175.-t*.51)+.13*sin(uv.y*283.-t*.317+uv.x*90.);
    c.rgb+=lava*vec3(.11,.037,.004)*hot*(.5+.5*darkness)*(.8+.2*heat);
    // Almost all lamps are steady. Sparse bright cores vary independently.
    float seed=hash(floor(uv*vec2(150,120)));
    float lamps=smoothstep(.81,.99,base.r)*smoothstep(.26,.5,base.r-base.b)
        *step(.91,seed)*smoothstep(.3,.35,uv.y)*(1.0-smoothstep(.65,.7,uv.y));
    c.rgb*=1.0+lamps*darkness*.025*sin(t*(.19+seed*.17)+seed*53.);
    // Stars have independent phases; the stable moon is excluded.
    float stars=sky*(1.-moon)*smoothstep(.42,.7,base.b)*smoothstep(.11,.2,base.b-base.r)*step(.82,seed);
    c.rgb*=1.0+stars*darkness*.05*sin(t*(.13+seed*.23)+seed*61.);
    fragColor=c*qt_Opacity;
}
