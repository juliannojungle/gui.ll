#!/bin/bash

PACKAGES=""

dpkg -s libsdl2-dev >/dev/null 2>&1 || PACKAGES="libsdl2-dev"
dpkg -s gdb >/dev/null 2>&1 || PACKAGES="$PACKAGES gdb"

if [ -z "$PACKAGES" ]; then
    echo "All dependencies already installed."
else
    echo "Installing:$PACKAGES"
    sudo apt-get update
    sudo apt-get install -y $PACKAGES
fi

if ! grep -q "set debuginfod enabled off" ~/.gdbinit 2>/dev/null; then
    echo "set debuginfod enabled off" >> ~/.gdbinit
    echo "GDB: disabled debuginfod in ~/.gdbinit"
fi
