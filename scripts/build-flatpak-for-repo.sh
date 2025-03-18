#!/bin/sh

flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo &&
flatpak-builder build -vv --user --force-clean --install-deps-from=flathub zone.xiv.novus.yml --gpg-sign=0xD28B9141A3B3A73A --repo=/home/josh/sources/flatpak-distrib &&
flatpak build-update-repo -vv --gpg-sign=0xD28B9141A3B3A73A --generate-static-deltas --prune /home/josh/sources/flatpak-distrib
