#version 330 core
out vec4 color;

in vec4 vertex_col;
in vec3 vertNoxA;

uniform vec3 lightPos;

in vec3 fragPos;
in vec2 vertex_tex;

uniform sampler2D tex;

void main()
{
    color = vertex_col;
    vec3 norm = normalize(vertNoxA);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1, 1, 1);
    if (true){ //tex
        vec4 t_col = texture(tex, vertex_tex);
        color = t_col;
    }
    float intensity = dot(normalize(lightDir), norm);
    vec4 color2;
    if (intensity > 0.95)      color2 = vec4(1.0, 1.0, 1.0, 1.0);
    else if (intensity > 0.75) color2 = vec4(0.8, 0.8, 0.8, 1.0);
    else if (intensity > 0.50) color2 = vec4(0.6, 0.6, 0.6, 1.0);
    else if (intensity > 0.25) color2 = vec4(0.4, 0.4, 0.4, 1.0);
    else                       color2 = vec4(0.2, 0.2, 0.2, 1.0);

    vec3 result = (vec3(0, 0, 0) + diffuse) * color.xyz;
    color = vec4(result, vertex_col.a) * color2;
}
