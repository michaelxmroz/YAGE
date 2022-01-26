#version 450

#if VERTEX_SHADER

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;

layout(location = 0) out vec2 outUv;


void main() {
    gl_Position = vec4(inPosition, 1.0);
    outUv = inUv;
}

#endif

#if FRAGMENT_SHADER


layout(location = 0) in vec2 outUv;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D mainTex;

void main() {
    //outColor = vec4(outUv.xy, 0.0, 1.0);
    outColor = texture(mainTex, outUv.xy);
}

#endif