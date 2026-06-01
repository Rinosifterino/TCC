#include "raylib.h"

Shader LoadLambertShader(void)
{
    const char *vertexShader =
        "#version 330\n"

        "in vec3 vertexPosition;\n"
        "in vec2 vertexTexCoord;\n"
        "in vec3 vertexNormal;\n"
        "in vec4 vertexColor;\n"

        "uniform mat4 mvp;\n"

        "out vec2 fragTexCoord;\n"
        "out vec4 fragColor;\n"
        "out vec3 fragNormal;\n"

        "void main()\n"
        "{\n"
        "    fragTexCoord = vertexTexCoord;\n"
        "    fragColor = vertexColor;\n"

        "    fragNormal = normalize(vertexNormal);\n"

        "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
        "}\n";

    const char *fragmentShader =
        "#version 330\n"

        "in vec2 fragTexCoord;\n"
        "in vec4 fragColor;\n"
        "in vec3 fragNormal;\n"

        "uniform vec3 lightDirection;\n"
        "uniform vec4 colDiffuse;\n"
        "uniform sampler2D texture0;\n"

        "out vec4 finalColor;\n"

        "void main()\n"
        "{\n"

        "    vec3 normal = normalize(fragNormal);\n"
        "    vec3 lightDir = normalize(lightDirection);\n"

        "    float diffuse = max(dot(normal, lightDir), 0.0);\n"

        "    float ambient = 0.20;\n"

        "    vec4 albedo = texture(texture0, fragTexCoord)"
        "                 * fragColor"
        "                 * colDiffuse;\n"

        "    vec3 lighting = albedo.rgb * (ambient + diffuse);\n"

        "    finalColor = vec4(lighting, albedo.a);\n"

        "}\n";

    return LoadShaderFromMemory(
        vertexShader,
        fragmentShader
    );
}