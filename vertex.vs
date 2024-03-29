#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 lightDir;
out vec3 Normal;
uniform vec3 lightPos; 
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec3 FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    lightDir = normalize(lightPos - FragPos);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}