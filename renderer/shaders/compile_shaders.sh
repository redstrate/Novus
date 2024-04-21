#!/bin/sh
# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

glslc mesh.vert -o mesh.vert.spv &&
glslc skinned.vert -o skinned.vert.spv &&
glslc mesh.frag -o mesh.frag.spv &&
glslc imgui.vert -o imgui.vert.spv &&
glslc imgui.frag -o imgui.frag.spv &&
glslc dummy.frag -o dummy.frag.spv &&
glslc blit.vert -o blit.vert.spv &&
glslc blit.frag -o blit.frag.spv