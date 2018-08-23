#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
launchctl unload /Library/LaunchDaemons/com.hieplpvip.AsusFnKeysDaemon.plist 2>/dev/null
sudo cp $DIR/com.hieplpvip.AsusFnKeysDaemon.plist /Library/LaunchDaemons
sudo cp $DIR/AsusFnKeysDaemon /usr/bin
launchctl load /Library/LaunchDaemons/com.hieplpvip.AsusFnKeysDaemon.plist
