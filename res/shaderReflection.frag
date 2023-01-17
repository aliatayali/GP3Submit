#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{             
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    
    vec4 color;
	color = vec4(0.3,0.3,0.3,0.5);
    
    vec4 mapping;
    mapping = vec4(texture(skybox, R).rgb, 1.0);
   
    FragColor.rgb = mix(color.rgb,mapping.rgb, 0.7);
}