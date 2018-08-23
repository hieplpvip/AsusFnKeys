#!/bin/bash

launchctl unload /Library/LaunchDaemons/com.hieplpvip.AsusFnKeysDaemon.plist 2>/dev/null
sudo cp ./build/Debug/com.hieplpvip.AsusFnKeysDaemon.plist /Library/LaunchDaemons
sudo cp ./build/Debug/AsusFnKeysDaemon /usr/bin
launchctl load /Library/LaunchDaemons/com.hieplpvip.AsusFnKeysDaemon.plist
