#version 120
uniform sampler2D texture;
uniform vec2 resolution;
uniform float blurRadius;

void main() {
    vec2 st = gl_FragCoord.xy / resolution.xy;
    vec4 color = vec4(0.0);
    vec2 texelSize = 1.0 / resolution;
    
    float totalWeight = 0.0;
    for (float x = -4.0; x <= 4.0; x += 1.0) {
        for (float y = -4.0; y <= 4.0; y += 1.0) {
            float weight = exp(-(x*x + y*y) / (2.0 * blurRadius * blurRadius));
            vec2 offset = vec2(x, y) * texelSize * (blurRadius * 0.5);
            color += texture2D(texture, st + offset) * weight;
            totalWeight += weight;
        }
    }
    
    gl_FragColor = color / totalWeight;
}