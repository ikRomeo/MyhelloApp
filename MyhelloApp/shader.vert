#version 450

layout(location = 0) in vec3 inColor; // Declare input for color
layout(location = 0) out vec3 fragColor; // Declare output for color

// Correctly define the array of positions
vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

void main() {
    // Use positions array with gl_VertexIndex
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = inColor; // Pass the input color to the fragment shader
}
