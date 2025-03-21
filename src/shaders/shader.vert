#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

layout (binding = 0) uniform ViewProjection {
    mat4 view;
    mat4 projection;
} viewProjection;

layout (push_constant) uniform PushModel {
    mat4 model;
} pushModel;

layout (location = 0) out vec4 vertexColor;

void main() {
    gl_Position = viewProjection.projection * viewProjection.view * pushModel.model * vec4(position, 1.0);
    vertexColor = vec4(color, 1.0);
}
