// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Arjen Hiemstra <ahiemstra@heimr.nl>

#version 440

layout(std140, binding = 0) uniform buf {
    highp mat4 matrix; // 16 components
    mediump vec4 viewport; // 20 components
    mediump float opacity; // 21 components
} ubuf;

layout(location = 0) in highp vec4 in_vertex;
layout(location = 1) in highp vec2 in_uv0;

layout(location = 0) out highp vec2 uv0;

out gl_PerVertex { vec4 gl_Position; };

void main() {
    uv0 = in_uv0;

    highp vec4 position = ubuf.matrix * in_vertex;

    // Pixel alignment correction. This converts from clip space to viewport
    // space, rounds the resulting pixel value, then converts back to clip
    // space. This ensures that the vertices are always aligned to exact pixels,
    // which avoids rendering artifacts in textures.
    highp vec2 pixel = ((position.xy / position.w) * 0.5) + 0.50001;
    pixel = round(pixel * ubuf.viewport.zw) / ubuf.viewport.zw;
    position.xy = (pixel * 2.0 - 1.0) * position.w;

    gl_Position = position;
}
