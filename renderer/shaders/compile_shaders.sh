#!/bin/sh
# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

glslc mesh.vert -o mesh.vert.spv &&
glslc mesh.frag -o mesh.frag.spv