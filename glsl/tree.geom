#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 16) out;

// Used in the vertex transformation
uniform mat4 uMVMatrix;
uniform mat4 uPMatrix;

uniform sampler2D uHeightmap;
uniform float uSize;

out vec2 vTexCoords;
out float linearZ;

const float PI = 3.14159265257;

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 qtransform(vec4 q, vec3 v) {
    return v + 2.0 * cross(cross(v, q.xyz) - q.w*v, q.xyz);
}

vec4 quarternionFromAxisAndAngle(vec3 axis, float angle) {
    float qx = axis.x * sin(angle/2);
    float qy = axis.y * sin(angle/2);
    float qz = axis.z * sin(angle/2);
    float qw = cos(angle/2);
    return vec4(qx, qy, qz, qw);
}

vec4 transform(vec3 pos) {
    vec4 noproj = uMVMatrix * vec4(pos, 1.0);
    linearZ = (-noproj.z - 1.0) / (10000.0 - 1.0);
    return uPMatrix * noproj;
}

void createFace(float angle, float displacement, float height, vec3 pos) {
    vec4 q = quarternionFromAxisAndAngle(vec3(0.0, 1.0, 0.0), angle);
    
    vec3 pt1 = vec3( 0.75, 0.0, 0.0);
    vec3 pt2 = vec3( 0.75, 1.0, 0.0);
    vec3 pt3 = vec3(-0.75, 0.0, 0.0);
    vec3 pt4 = vec3(-0.75, 1.0, 0.0);

    vec3 tpt1 = qtransform(q, pt1) + qtransform(q, vec3(0.0, 0.0, -displacement)); tpt1.xz *= uSize; tpt1.y *= height;
    gl_Position = transform(tpt1 + pos); 
    vTexCoords = vec2(0.0, 1.0);
    EmitVertex();

    vec3 tpt2 = qtransform(q, pt2) + qtransform(q, vec3(0.0, 0.0, -displacement)); tpt2.xz *= uSize; tpt2.y *= height;
    gl_Position = transform(tpt2 + pos); 
    vTexCoords = vec2(0.0, 0.0);
    EmitVertex();

    vec3 tpt3 = qtransform(q, pt3) + qtransform(q, vec3(0.0, 0.0, -displacement)); tpt3.xz *= uSize; tpt3.y *= height;
    gl_Position = transform(tpt3 + pos); 
    vTexCoords = vec2(1.0, 1.0);
    EmitVertex();

    vec3 tpt4 = qtransform(q, pt4) + qtransform(q, vec3(0.0, 0.0, -displacement)); tpt4.xz *= uSize; tpt4.y *= height;
    gl_Position = transform(tpt4 + pos); 
    vTexCoords = vec2(1.0, 0.0);
    EmitVertex();

    EndPrimitive();
}

float terrainHeight(vec2 uv) {
    float coarse = texture(uHeightmap, uv).x * 3200.0 - 1600.0;
    return coarse;
}

void main(void) {

    vec3 pos = gl_in[0].gl_Position.xyz;
    float height = pos.y;
    vec2 uv = pos.xz / 16384.0 - vec2(0.5, 0.5);
    float posy = terrainHeight(uv);

    // do not generate tree under water
    if (posy >= 0.0 && texture(uHeightmap, uv).x < 0.7) {
        pos = vec3(pos.x, posy, pos.z);
        float angle = rand(pos.zx);

        createFace(angle, 0.0, height, pos); angle += PI / 4.0;
        createFace(angle, 0.0, height, pos); angle += PI / 4.0;
        createFace(angle, 0.0, height, pos); angle += PI / 4.0;
        createFace(angle, 0.0, height, pos); 
    }
}
