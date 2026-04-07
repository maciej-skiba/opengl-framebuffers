#version 330 core
out vec4 FragColor;  
in vec3 ourColor;
in vec4 FragPos;

uniform vec4 cameraPos;

void main()
{
    float dist = length(cameraPos - FragPos);
    float density = 0.006;
    float fogFactor = exp(-density * dist);
    fogFactor = clamp(fogFactor, 0.0, 1.0);


    vec3 fogColor = vec3(0.69, 0.76, 0.82);

    vec3 finalColor = mix(fogColor, ourColor, fogFactor);
    FragColor = vec4(finalColor.rgb, 1.0f);
}