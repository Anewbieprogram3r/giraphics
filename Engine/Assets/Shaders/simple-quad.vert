#version 450

const float dim = 0.5;
vec2 positions[4] = vec2[]
(
    vec2(-dim, -dim),
    vec2 (dim,-dim),
    vec2 (-dim, dim),
    vec2 (dim, dim)
);
vec4 colors[4] = vec4[]
(
    vec4(1.0f, 0.0, 0.0, 1.0),
    vec4(0.0f, 1.0, 0.0, 1.0),
    vec4(0.0f, 0.0, 1.0, 1.0),
    vec4(1.0f, 1.0, 1.0, 1.0)
);
layout(location=0) out vec4 outColor;
void main()
{
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0f);
    outColor = colors [gl_VertexIndex];
}