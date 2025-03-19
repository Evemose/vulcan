#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

layout (binding = 0) uniform ViewProjection {
    mat4 view;
    mat4 model;
} mvp;

layout (location = 0) out vec4 vertexColor;

void main() {
    gl_Position = mat4(1.0) * mvp.view * mvp.model * vec4(position, 1.0);
    vertexColor = vec4(color, 1.0);
}
