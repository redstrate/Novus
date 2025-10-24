# Renderer

This handles rendering FFXIV visual data, such as models and materials.

Currently it has a basic rendering method enabled by default, which is limited to displaying a material's diffuse texture and not much else.

## Experimental

There's a more advanced renderer which is still in development, which uses the game's own shaders to render models. Launch any Novus tool with `NOVUS_USE_NEW_RENDERER=1` set to enable it.

**NOTE:** We only support this mode for modern game versions (Dawntrail and above.) If you want to view game assets in an older version, you will have to use the basic rendering mode.
