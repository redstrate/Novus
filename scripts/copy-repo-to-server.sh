#!/bin/sh

rsync -e "ssh -p 38901 -o StrictHostKeyChecking=no" --recursive /home/josh/sources/flatpak-distrib/ ryne.moe:/srv/http/astra-distrib/flatpak
