//Version number
#version 400
//Layout Qualifer
layout( location = 0 ) out vec4 fragcolor;

//Tcs
in vec2 TexCoords;
uniform sampler2D diffuse;

 
void main()
{
//Setting each vector component to uniform varaible then setting final colour
	vec4 color;
	color = texture2D(diffuse, TexCoords);
    fragcolor = color;
}