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
        "uniform mat4 matModel;\n"
        "uniform mat4 matNormal;\n"

        "out vec2 fragTexCoord;\n"
        "out vec4 fragColor;\n"
        "out vec3 fragNormal;\n"
        "out vec3 fragPosition;\n"

        "void main()\n"
        "{\n"
        "    fragTexCoord = vertexTexCoord;\n"
        "    fragColor = vertexColor;\n"

        "    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));\n"
        "    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));\n"

        "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
        "}\n";

    const char *fragmentShader =
        "#version 330\n"

        "in vec2 fragTexCoord;\n"
        "in vec4 fragColor;\n"
        "in vec3 fragNormal;\n"
        "in vec3 fragPosition;\n"

        "uniform vec3 lightDirection;\n"
        "uniform vec4 colDiffuse;\n"
        "uniform sampler2D texture0;\n"
        "uniform mat4 lightVP;\n"
        "uniform sampler2D shadowMap;\n"
        "uniform int useShadows;\n"
        "uniform int shadowMapResolution;\n"

        "out vec4 finalColor;\n"

        "float CalculateShadow(vec3 fragPosWorld, vec3 normal, vec3 lightDir)\n"
        "{\n"
        "    vec4 fragPosLightSpace = lightVP * vec4(fragPosWorld, 1.0);\n"
        "    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;\n"
        "    projCoords = projCoords * 0.5 + 0.5;\n"
        "    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)\n"
        "        return 0.0;\n"
        "    float bias = max(0.0025 * (1.0 - dot(normal, lightDir)), 0.0005);\n"
        "    float currentDepth = projCoords.z;\n"
        "    vec2 texelSize = 1.0 / vec2(float(shadowMapResolution));\n"
        "    int shadowCounter = 0;\n"
        "    for (int x = -1; x <= 1; x++)\n"
        "    {\n"
        "        for (int y = -1; y <= 1; y++)\n"
        "        {\n"
        "            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;\n"
        "            if (currentDepth - bias > pcfDepth) shadowCounter++;\n"
        "        }\n"
        "    }\n"
        "    return float(shadowCounter) / 9.0;\n"
        "}\n"

        "void main()\n"
        "{\n"

        "    vec3 normal = normalize(fragNormal);\n"
        "    vec3 lightDir = normalize(lightDirection);\n"

        "    float diffuse = max(dot(normal, lightDir), 0.0);\n"
        "    float shadow = (useShadows == 1) ? CalculateShadow(fragPosition, normal, lightDir) : 0.0;\n"
        "    diffuse *= (1.0 - shadow);\n"

        "    float ambient = 0.20;\n"

        "    vec4 albedo = texture(texture0, fragTexCoord)"
        "                 * fragColor"
        "                 * colDiffuse;\n"

        "    float lightIntensity = min(ambient + diffuse * (1.0 - ambient), 1.0);\n"
        "    vec3 lighting = albedo.rgb * lightIntensity;\n"

        "    finalColor = vec4(lighting, albedo.a);\n"

        "}\n";

    return LoadShaderFromMemory(
        vertexShader,
        fragmentShader
    );
}
