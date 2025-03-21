#version 330 core

out vec4 fragColor;

void main() {
    float intensity = abs(sin(gl_FragCoord.x * 0.1)); // Flashing effect based on x position
    fragColor = vec4(intensity, 0.0, 0.0, 1.0); // Red color with varying intensity
}
