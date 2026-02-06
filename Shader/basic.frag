#version 330 core
out vec4 FragColor;

in vec3 chNormal;
in vec3 chFragPos;
in vec2 chUV;

uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;
uniform float uLightIntensity;
uniform vec3 uMaterialColor;
uniform bool uUseTexture;

uniform sampler2D uDiffMap1;

void main()
{
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * uLightColor;

    // diffuse
    vec3 norm = normalize(chNormal);
    vec3 lightDir = normalize(uLightPos - chFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor * uLightIntensity;

    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(uViewPos - chFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * uLightColor * uLightIntensity;

    vec3 objectColor;
    if (uUseTexture) {
        objectColor = texture(uDiffMap1, chUV).rgb;
    } else {
        objectColor = uMaterialColor;
    }

    FragColor = vec4(objectColor * (ambient + diffuse + specular), 1.0);
}

