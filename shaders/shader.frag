#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;    // Input color from vertex shader
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;     // Output color

void main() {
    outColor = vec4(fragColor * texture(texSampler, fragTexCoord).rgb, 1.0);         // Set the output color
}
