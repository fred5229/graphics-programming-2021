#version 330 core

out vec4 fragColor;

// TODO 2.6: should receive the age of the particle as an input variable
in float elapsedTimeFrag;

const vec3 startCol = vec3(1.0, 1.0, 0.05);
const vec3 midCol = vec3(1.0, 0.5, 0.01);
const vec3 endCol = vec3(0.0, 0.0, 0.0);

const float midAge = 5.0;
const float maxAge = 10.0;

void main()
{
        // TODO 2.4 set the alpha value to 0.2 (alpha is the 4th value of the output color)
    vec2 vecFromCenter = (gl_PointCoord - vec2(.5, .5)) * 2;
    float distance = sqrt(dot(vecFromCenter, vecFromCenter));

    // TODO 2.5 and 2.6: improve the particles appearance
    vec3 color = mix(startCol, midCol, elapsedTimeFrag / midAge);
    if (elapsedTimeFrag > midAge){
        color = mix(midCol, endCol, (elapsedTimeFrag - midAge) / midAge);
    }

    float alpha = mix(1.0 - distance, 0, elapsedTimeFrag / maxAge);

    // remember to replace the default output (vec4(1.0,1.0,1.0,1.0)) with the color and alpha values that you have computed
    fragColor = vec4(color, alpha);

}