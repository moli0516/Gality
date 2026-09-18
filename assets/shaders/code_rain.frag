#version 120

// ============================================================================
// Code Rain Shader — Gality Engine Title Menu
//
// Simulates a "Matrix-style" falling code effect overlaid on the title
// background image.
//
// Uniforms:
//   u_texture    — The background image texture
//   u_resolution — Screen resolution (vec2)
//   u_time       — Elapsed time in seconds (float)
//   u_intensity  — Overall brightness multiplier (0.0 ~ 1.0)
//   u_color      — Base color of the falling code (vec3, 0.0 ~ 1.0)
// ============================================================================

uniform sampler2D u_texture;
uniform vec2 u_resolution;
uniform float u_time;
uniform float u_intensity;
uniform vec3 u_color;

// ----------------------------------------------------------------------------
// Hash functions for pseudo-random values
// ----------------------------------------------------------------------------
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

vec2 hash2(vec2 p) {
    return fract(sin(vec2(
        dot(p, vec2(127.1, 311.7)),
        dot(p, vec2(269.5, 183.3))
    )) * 43758.5453);
}

// ----------------------------------------------------------------------------
// Grid size: number of columns and rows in the code rain
// ----------------------------------------------------------------------------
#define COLUMNS 48.0
#define ROWS 32.0

void main() {
    // UV coordinates
    vec2 uv = gl_TexCoord[0].xy;

    // Sample base background
    vec4 baseColor = texture2D(u_texture, uv);

    // ------------------------------------------------------------------------
    // Grid cell coordinates
    // ------------------------------------------------------------------------
    vec2 gridUV = uv * vec2(COLUMNS, ROWS);
    vec2 cell = floor(gridUV);
    vec2 cellUV = fract(gridUV);

    // ------------------------------------------------------------------------
    // Column-level properties (each column falls at different speed)
    // ------------------------------------------------------------------------
    float columnSeed = hash(vec2(cell.x, 0.0));
    float columnSpeed = 0.4 + columnSeed * 0.8;      // 0.4 ~ 1.2
    float columnOffset = columnSeed * 100.0;         // phase offset

    // Vertical position in this column
    float fallY = gridUV.y + u_time * columnSpeed + columnOffset;

    // ------------------------------------------------------------------------
    // Per-cell properties
    // ------------------------------------------------------------------------
    float cellSeed = hash(cell + floor(u_time * 3.0) * 0.01);

    // Glyph-like rectangle inside each cell (not full cell)
    // Creates the appearance of "characters" rather than a solid block
    float glyphMask = 0.0;
    if (cellUV.x > 0.15 && cellUV.x < 0.85 &&
        cellUV.y > 0.20 && cellUV.y < 0.80) {
        glyphMask = 1.0;
    }

    // Add internal variation to make it look like a character
    vec2 glyphLocal = (cellUV - 0.5) / 0.35;
    float glyphPattern = step(0.3, hash(floor(glyphLocal * 3.0 + cellSeed * 10.0)));
    glyphMask *= (0.6 + 0.4 * glyphPattern);

    // ------------------------------------------------------------------------
    // Trail effect (falling column has leading edge + trail)
    // ------------------------------------------------------------------------
    float columnPhase = fract(fallY / ROWS + columnSeed);
    float leadBrightness = smoothstep(0.0, 0.05, columnPhase) *
                           (1.0 - smoothstep(0.05, 0.5, columnPhase));

    // The trailing tail decays slowly
    float trailBrightness = exp(-columnPhase * 3.0);

    // Combine lead and trail
    float codeBrightness = max(leadBrightness, trailBrightness * 0.4);

    // Randomly turn off some cells to break up the pattern
    float flicker = step(0.15, cellSeed);
    codeBrightness *= flicker;

    // Apply the glyph mask
    codeBrightness *= glyphMask;

    // Apply global intensity
    codeBrightness *= u_intensity;

    // ------------------------------------------------------------------------
    // Slight horizontal glow bleed (optional, adds bloom-like feel)
    // ------------------------------------------------------------------------
    float glow = 0.0;
    for (float dx = -1.0; dx <= 1.0; dx += 1.0) {
        vec2 neighborCell = cell + vec2(dx, 0.0);
        float neighborSeed = hash(neighborCell + floor(u_time * 3.0) * 0.01);
        float neighborFlicker = step(0.15, neighborSeed);
        glow += neighborFlicker * 0.1;
    }

    // ------------------------------------------------------------------------
    // Final composition
    // ------------------------------------------------------------------------
    vec3 codeColor = u_color * codeBrightness;
    vec3 glowColor = u_color * glow * u_intensity * 0.3;

    vec3 finalColor = baseColor.rgb + codeColor + glowColor;

    // Preserve original alpha (background stays opaque)
    gl_FragColor = vec4(finalColor, baseColor.a);
}