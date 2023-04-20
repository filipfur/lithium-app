#version 460

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 1) uniform image2D image;

uniform sampler2D u_noise_texture;

const vec2 resolution = vec2(2048.0);
const vec3 lightPosition = vec3(1.5, 4.5, -5.0);
const int maxBounces = 5;

const vec4 spheres[5] = vec4[5](
    vec4(-1.0, 0.5, -3.0, 0.5), 
    vec4(1.0, 0.5, -4.0, 0.5), 
    vec4(0.0, 0.5, -5.0, 0.5), 
    vec4(0.0, -128.0, 0.0, 128.0), 
    vec4(lightPosition, 0.5));

const vec3 colors[5] = vec3[5](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 1.0, 1.0),
    vec3(1.0, 1.0, 1.0));

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

bool testSphere(in vec3 rayOrigo, in vec3 rayDir, in vec3 spherePos, in float sphereRadius, inout float t)
{
    vec3 sphereToEye = rayOrigo - spherePos;
    float b = dot(rayDir, sphereToEye);
    float c = dot(sphereToEye, sphereToEye) - sphereRadius * sphereRadius;
    float bsqmc = b * b - c;
    t = -b - sqrt(bsqmc);

    return bsqmc >= 0.0;
}

bool testCube(in vec3 rayOrigo, in vec3 rayDir, in vec3 spherePos, in float sphereRadius, inout float t)
{
    vec3 sphereToEye = rayOrigo - spherePos;
    float b = dot(rayDir, sphereToEye);
    float c = dot(sphereToEye, sphereToEye) - sphereRadius * sphereRadius;
    float bsqmc = b * b - c;
    t = -b - sqrt(bsqmc);

    return bsqmc >= 0.0;
}

vec3 lightning(in vec3 rayOrigo, in vec3 rayDir, in vec3 spherePos, in float sphereRadius, in float t, in vec3 sphereColor)
{
    vec3 intersection = rayOrigo + t * rayDir;
    vec3 normal = normalize(intersection - spherePos);

    vec3 lightDir = normalize(lightPosition - intersection);
    //vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diffuse = max(dot(normal, lightDir), 0.0);

    vec3 F0 = vec3(0.04); // Fresnel reflectance at normal incidence for dielectric materials
    float cosTheta = dot(normal, -rayDir);
    vec3 fresnel = fresnelSchlick(cosTheta, F0);

    // blinn-phon lightning
    vec3 viewDir = normalize(-rayDir);
    vec3 halfDir = normalize(lightDir + viewDir);
    float specular = pow(max(dot(normal, halfDir), 0.0), 32.0);

    float ambient = 0.2;

    return sphereColor * ambient + sphereColor * (diffuse + specular + fresnel);
}

vec3 calculateLighting(vec3 normal, vec3 viewDirection, vec3 lightDirection, vec3 lightColor, vec3 baseColor)
{
    // Diffuse lighting
    float diffuseFactor = max(dot(normal, lightDirection), 0.0);
    vec3 diffuseColor = diffuseFactor * lightColor * baseColor;

    // Specular lighting
    vec3 halfwayVector = normalize(lightDirection + viewDirection);
    float specularFactor = max(dot(normal, halfwayVector), 0.0);
    float shininess = 32.0;
    vec3 specularColor = lightColor * pow(specularFactor, shininess);

    /*vec3 F0 = vec3(0.04); // Fresnel reflectance at normal incidence for dielectric materials
    float cosTheta = dot(normal, viewDirection);
    vec3 fresnel = fresnelSchlick(cosTheta, F0);*/

    return diffuseColor + specularColor;// + fresnel * 0.1;
}

vec3 rotateVector(vec3 v, vec3 axis, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return v * c + cross(axis, v) * s + axis * dot(axis, v) * oc;
}

void main()
{
    vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);

    float fov = 45.0;
    vec3 cameraPosition = vec3(0.0, 4.0, -1.0);
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec4 noise = texture(u_noise_texture, vec2(pixelCoord) / resolution);

    vec2 ndc = (2.0 * vec2(pixelCoord) - resolution) / resolution;
    float aspectRatio = resolution.x / resolution.y;
    float tanHalfFOV = tan(radians(fov) * 0.5);

    vec3 rayDir;
    rayDir.x = ndc.x * aspectRatio * tanHalfFOV;
    rayDir.y = ndc.y * tanHalfFOV;
    rayDir.z = -1.0;
    rayDir = normalize(rayDir);

    rayDir = rotateVector(rayDir, vec3(1.0, 0.0, 0.0), radians(-55.0));

    vec3 rayOrigo = cameraPosition;

    vec3 accumulatedColor = vec3(0.0);
    vec3 reflectance = vec3(1.0);

    for(int bounceCount = 0; bounceCount < maxBounces; bounceCount++)
    {
        float closestIntersection = 1e10;
        int n = -1;

        for(int i = 0; i < 4; ++i)
        {
            float t;
            if(testSphere(rayOrigo, rayDir, spheres[i].xyz, spheres[i].w, t) && t < closestIntersection)
            {
                closestIntersection = t;
                n = i;
            }
        }
        if(n == -1)
        {
            break;
        }
        vec3 intersection = rayOrigo + closestIntersection * rayDir;
        vec3 normal = normalize(intersection - spheres[n].xyz);

        vec3 reflectedDir = reflect(-rayDir, normal);
        vec3 baseColor = colors[n];

        vec3 lightDir = normalize(lightPosition - intersection);

        accumulatedColor += reflectance * (baseColor * 0.2 + calculateLighting(normal, -rayDir, lightDir, vec3(1.0), baseColor));
        reflectance *= 0.5; // adjust this value for the desired reflectivity

        rayOrigo = intersection + 0.001 * reflectedDir;
        rayDir = reflectedDir;
    }

    pixel.rgb += accumulatedColor;

    /*float t = 0.0;
    if(testSphere(rayOrigo, rayDir, lightPosition, 0.5, t))
    {
        pixel.rgb = vec3(2.0, 2.0, 2.0);
    }*/

    // gamma correction
    //pixel.rgb = pow(pixel.rgb, vec3(1.0 / 2.2));
    imageStore(image, pixelCoord, pixel);
}