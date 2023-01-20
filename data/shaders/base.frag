#version 330 core

out vec4 o_color;

uniform vec3 u_view_pos;
uniform vec3 u_light_pos;

in vec3 v_position;
in vec3 v_normal;

void main()
{
    vec3 light_color = vec3(1, 1, 1);
    float ambient_str = 0.5;
    vec3 ambient = ambient_str * light_color;

    vec3 norm = normalize(v_normal);
    vec3 light_dir = normalize(u_light_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;
    
    float specular_str = 0.3;
    vec3 view_dir = normalize(u_view_pos - v_position);
    vec3 reflect_dir = reflect(-light_dir, norm);  
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = specular_str * spec * light_color;  
        
    vec3 result = (ambient + diffuse*0.4 + specular);
    o_color = vec4(result, 1.0);
} 