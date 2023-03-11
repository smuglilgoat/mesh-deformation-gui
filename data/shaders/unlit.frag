#version 330 core

out vec4 o_color;

uniform vec3 u_view_pos;
uniform vec3 u_light_pos;

in vec3 v_position;
in vec3 v_normal;

void main()
{
    o_color = vec4(1.0, 1.0, 1.0, 1.0);
} 