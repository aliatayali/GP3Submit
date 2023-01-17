#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;
in vec2 tcs;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform sampler2D diffuse;

void main()
{             
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    
    vec4 color;
	color = texture2D(diffuse, tcs);
    
    vec4 mapping;
    mapping = vec4(texture(skybox, R).rgb, 0.6);
   
    FragColor.rgb = mix(color.rgb,mapping.rgb, 0.4);
}