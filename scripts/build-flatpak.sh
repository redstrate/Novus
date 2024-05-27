#!/bin/sh

flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo &&
flatpak-builder build --user --force-clean --install-deps-from=flathub zone.xiv.novus.yml &&
flatpak build-export export build &&
flatpak build-bundle export novus.flatpak zone.xiv.novus --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo