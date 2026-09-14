#version 120

uniform sampler2D u_oldTexture;
uniform sampler2D u_newTexture;
uniform sampler2D u_maskTexture;
uniform float u_progress;   // 0.0 到 1.0
uniform float u_softness;   // 羽化值，建議 0.05 ~ 0.15

void main() {
    vec2 uv = gl_TexCoord[0].xy;

    vec4 oldColor = texture2D(u_oldTexture, uv);
    vec4 newColor = texture2D(u_newTexture, uv);
    float maskValue = texture2D(u_maskTexture, uv).r;

    // 透過 smoothstep 建立動態平滑閾值
    // 當 progress 推進時，低於 progress 的像素轉變為 newColor
    float edge = u_progress * (1.0 + u_softness);
    float alpha = smoothstep(edge - u_softness, edge, maskValue);

    // alpha == 1.0 顯示舊場景，alpha == 0.0 顯示新場景
    gl_FragColor = mix(newColor, oldColor, alpha);
}