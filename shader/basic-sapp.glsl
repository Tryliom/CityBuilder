#pragma sokol @vs vs
uniform vs_window
{
    vec4 screen_size;
};

in vec4 position;
in vec4 color0;
in vec2 uv0;

out vec4 color;
out vec2 uv;

void main()
{
    gl_Position = ((position * vec4(2, 2, 1, 1)) / vec4(screen_size.x, screen_size.y, 1, 1) - vec4(1, 1, 0, 0)) * vec4(1, -1, 1, 1);
    color = color0;
    uv = uv0;
}
#pragma sokol @end

#pragma sokol @fs fs
uniform sampler2D tex;

in vec4 color;
in vec2 uv;

out vec4 frag_color;

void main()
{
    if (uv == vec2(-1, -1))
    {
        frag_color = color;
    }
    else
    {
        frag_color = texture(tex, uv) * color;
    }
}

#pragma sokol @end

#pragma sokol @program ui vs fs
