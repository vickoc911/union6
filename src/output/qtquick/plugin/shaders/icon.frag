// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Arjen Hiemstra <ahiemstra@heimr.nl>

#version 440

layout(std140, binding = 0) uniform buf {
    highp mat4 matrix; // 16 components
    mediump vec4 viewport; // 20 components
    mediump float opacity; // 21 components
} ubuf;

layout(binding = 1) uniform sampler2D textureSource;

layout(location = 0) in highp vec2 uv0;
layout(location = 0) out mediump vec4 out_color;

void main() {
    mediump vec4 texture_color = texture(textureSource, uv0);
    out_color = texture_color * ubuf.opacity;
}
