# Novus

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

## argcracker

This can crack any SqexArg formatted string you throw at it. However there are two
caveats:

1. You must know the current TickCount() when you got the string, the easiest 
way is to run your victim program and then run argcracker right after.
2. You must know at least one known argument (such as `UserPath`) or the decrypted result 
will
3. be hard to find.

### Usage

`argcracker.exe [sqexarg string] [tick range] [known arg]`

`argcracker.exe //**sqex0003p8LrsXt9_m9RJAsGzXd66zb3SxeTqZdhV**// 1000 UserPath`
