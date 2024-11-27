#version 330 core
in vec3 FragPos;

out vec4 FragColor;

uniform vec4 gridColor;

void main() {
    vec4 outColor = gridColor;
    if (FragPos.z == 0.0 && FragPos.x == 0.0) {
        outColor.y = 1.0;
    }
    else if (FragPos.x == 0.0) {
        outColor.z = 1.0;
    }
    else if (FragPos.z == 0.0) {
        outColor.x = 1.0;
    }
    FragColor = outColor;
}
