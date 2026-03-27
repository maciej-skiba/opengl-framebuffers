#version 330 core
out vec4 FragColor;  
in vec2 texCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;  

void main()
{
    FragColor = vec4(1.0 - texture(screenTexture, texCoords).rgb, 1.0);
}  