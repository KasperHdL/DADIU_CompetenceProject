#version 400
layout(location = 0)in vec3 position;
layout(location = 1)in vec3 normal;
layout(location = 2)in vec4 uv;
layout(location = 3)in vec4 color;

out vec3 vNormal;
out vec2 vUV;
out vec3 vEyePos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMat;

void main(void) {
    vec4 eyePos = view * model * vec4(position,1);
    gl_Position = projection * eyePos;

    vNormal = normalMat * normal;
    vUV = uv.xy;
    vEyePos = eyePos.xyz;
}
