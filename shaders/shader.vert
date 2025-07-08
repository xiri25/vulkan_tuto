#version 450

layout(location = 0) in vec2 inPosition; // Input position
layout(location = 1) in vec3 inColor;    // Input color

layout(location = 0) out vec3 fragColor;  // Output color to fragment shader

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0); // Set the position
    fragColor = inColor;                      // Pass color to fragment shader
}
