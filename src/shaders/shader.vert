#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

layout (binding = 0) uniform ViewProjection {
    mat4 view;
    mat4 projection;
} viewProjection;

layout (binding = 1) uniform Model {
    mat4 model;
} model;

layout (location = 0) out vec4 vertexColor;

void main() {
    gl_Position = viewProjection.projection * viewProjection.view * model.model * vec4(position, 1.0);
    vertexColor = vec4(color, 1.0);
}
