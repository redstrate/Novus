# Novus

A suite of unofficial FFXIV tools, including a model viewer and data archive explorer.

![Screenshot of some of the applications](https://xiv.zone/novus.png)

## Components

Here is an exhaustive list of tooling available here:

* [Argcracker](apps/argcracker), a program that can help decrypt game arguments.
* [Gear Editor](apps/armoury), a graphical gear and character viewer. It also supports GLTF import & export.
* [Map Editor](apps/mapeditor), a graphical map viewer.
* [Excel Editor](apps/karuku), a graphical program to view Excel data sheets.
* [Model Viewer](apps/mdlviewer), a graphical model viewer for MDL files.
* [Data Viewer](apps/sagasu), a graphical interface to explore FFXIV data archive files.
* [Material Editor](apps/mateditor), a program to view material files.
* [Game Launcher](apps/gamelauncher), a program to quickly launch the game for testing.

## Usage

There are no releases at the moment, but experimental builds are currently available for Windows and Linux via [GitHub Actions](https://github.com/redstrate/Novus/actions). Note that the Linux binaries may not be completely portable.

### Supported Game Versions

I have only tested Novus on 6.x (Endwalker) so far, and it has limited support for 7.x (Dawntrail) except for the new renderer. If you're using Novus against Dawntrail, make sure to set the `NOVUS_IS_DAWNTRAIL=1` environment variable.

## Building

Please refer to the [building document](BUILDING.md) for instructions on how to build Novus.

## Contributing & Support

The best way you can help is by [monetarily supporting me](https://redstrate.com/fund/) or by submitting patches to
help fix bugs or add functionality. Filing issues is appreciated, but I do this in my free time so please don't expect professional support.

## License

![GPLv3](https://www.gnu.org/graphics/gplv3-127x51.png)

This project is licensed under the [GNU General Public License 3](LICENSE). Some code or assets may be licensed differently, please refer to the [REUSE](https://reuse.software/spec/) metadata.
