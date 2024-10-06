#version 330

out vec4 color;

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 size;
const float samples = 12.0f;
const float quality = 6.0f;

void main() {
    vec4 sum = vec4(0.0f);
    vec2 sizeFactor = vec2(1.0f) / size * quality;
    vec4 source = texture(texture0, fragTexCoord);
    const int range = 5;

    for (int x = -range; x <= range; x++) {
        for (int y = -range; y <= range; y++) {
            sum += texture(texture0, fragTexCoord + vec2(x, y) * sizeFactor);
        }
    }

    color = ((sum / (samples * samples)) + source) * colDiffuse;
}