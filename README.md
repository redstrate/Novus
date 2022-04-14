# Novus

[![sourcehut](https://img.shields.io/badge/repository-sourcehut-lightgrey.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZlcnNpb249IjEuMSINCiAgICB3aWR0aD0iMTI4IiBoZWlnaHQ9IjEyOCI+DQogIDxkZWZzPg0KICAgIDxmaWx0ZXIgaWQ9InNoYWRvdyIgeD0iLTEwJSIgeT0iLTEwJSIgd2lkdGg9IjEyNSUiIGhlaWdodD0iMTI1JSI+DQogICAgICA8ZmVEcm9wU2hhZG93IGR4PSIwIiBkeT0iMCIgc3RkRGV2aWF0aW9uPSIxLjUiDQogICAgICAgIGZsb29kLWNvbG9yPSJibGFjayIgLz4NCiAgICA8L2ZpbHRlcj4NCiAgICA8ZmlsdGVyIGlkPSJ0ZXh0LXNoYWRvdyIgeD0iLTEwJSIgeT0iLTEwJSIgd2lkdGg9IjEyNSUiIGhlaWdodD0iMTI1JSI+DQogICAgICA8ZmVEcm9wU2hhZG93IGR4PSIwIiBkeT0iMCIgc3RkRGV2aWF0aW9uPSIxLjUiDQogICAgICAgIGZsb29kLWNvbG9yPSIjQUFBIiAvPg0KICAgIDwvZmlsdGVyPg0KICA8L2RlZnM+DQogIDxjaXJjbGUgY3g9IjUwJSIgY3k9IjUwJSIgcj0iMzglIiBzdHJva2U9IndoaXRlIiBzdHJva2Utd2lkdGg9IjQlIg0KICAgIGZpbGw9Im5vbmUiIGZpbHRlcj0idXJsKCNzaGFkb3cpIiAvPg0KICA8Y2lyY2xlIGN4PSI1MCUiIGN5PSI1MCUiIHI9IjM4JSIgc3Ryb2tlPSJ3aGl0ZSIgc3Ryb2tlLXdpZHRoPSI0JSINCiAgICBmaWxsPSJub25lIiBmaWx0ZXI9InVybCgjc2hhZG93KSIgLz4NCjwvc3ZnPg0KCg==)](https://git.sr.ht/~redstrate/novus)
[![GitHub
mirror](https://img.shields.io/badge/mirror-GitHub-black.svg?logo=github)](https://github.com/redstrate/novus)
[![ryne.moe
mirror](https://img.shields.io/badge/mirror-ryne.moe-red.svg?logo=git)](https://git.ryne.moe/redstrate/novus)

This is a collection of cross-platform, FFXIV modding tools. These don't use any pre-existing modding framework (Lumina,
xivModdingFramework, etc) but instead my own custom modding library, [libxiv](https://git.sr.ht/~redstrate/libxiv).

The goal is to create a good set of tools that isn't based on WPF and C#, and can work cross-platform without having
to resort to Wine.

**Note:** This is alpha level software, not intended to be usable in any capacity at the moment. Thus, I have not tagged
any stable releases.

## exdviewer

This is used to view excel data from the game. It's pretty basic right now but it seems to read anything I throw at it just fine.


![exdviewer screenshot](misc/exdviewer-screenshot.png)

### Usage

You must pass the path to your `sqpack` directory as the first argument.

`exdviewer.exe C:\Program Files (x86)\SquareEnix\Final Fantasy XIV\game\sqpack`

## mdlviewer

This can display any arbitrary model from the game, as long as it's supported by libxiv.

![mdlviewer screenshot](misc/mdlviewer-screenshot.png)

### Usage

You must pass the path to your `sqpack` directory as the first argument.

`mdlviewer.exe C:\Program Files (x86)\SquareEnix\Final Fantasy XIV\game\sqpack`

### Note

The viewport uses Vulkan, so it must be supported on your system in order to work.

If you're running mdlviewer on macOS (where Qt builds usually don't ship with MoltenVK unfortunatey)
mdlviewer will automatically reconfigure itself to use a standalone SDL2 window.

## explorer

This tool can list known files by libxiv, such as excel sheets.

### Usage

You must pass the path to your `sqpack` directory as the first argument.

`explorer.exe C:\Program Files (x86)\SquareEnix\Final Fantasy XIV\game\sqpack`
