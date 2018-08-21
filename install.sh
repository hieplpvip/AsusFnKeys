#!/bin/bash

sudo sh build.sh
sudo sh mount_efi.sh
sudo rm -rf /Volumes/EFI/EFI/CLOVER/kexts/Other/AsusFnKeys.kext
sudo cp -a ./build/Debug/AsusFnKeys.kext /Volumes/EFI/EFI/CLOVER/kexts/Other
sudo cp ./build/Debug/com.hieplpvip.AsusFnKeysDaemon.plist /Library/LaunchDaemons
sudo cp ./build/Debug/AsusFnKeysDaemon /usr/bin
launchctl unload /Library/LaunchDaemons/com.hieplpvip.AsusFnKeysDaemon.plist
launchctl load /Library/LaunchDaemons/com.hieplpvip.AsusFnKeysDaemon.plist
echo "Installed to EFI"
