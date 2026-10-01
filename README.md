# Dynamic Workspaces for Sway

DISCLAIMER: Claude Code was used in the development of this tool, but all code was reviewed (and refactored) by me.

This program lets you use Sway workspaces just like Niri or GNOME, where workspaces always maintain numbering from 0 to n, and moving beyond the last workspace creates a new one (instead of looping around like Sway does by default.)

## Installation

### Nix

Flake Coming Soon!

### Other Distros

Clone this repository and run the following:

```
make
```

Move `./build/dynwork` to your PATH.

## Usage

Add the following to your Sway Config:

```
exec dynwork daemon

bindgesture swipe:left exec echo left | socat - UNIX-CONNECT:$XDG_RUNTIME_DIR/dynwork.sock
bindgesture swipe:right exec echo right | socat - UNIX-CONNECT:$XDG_RUNTIME_DIR/dynwork.sock
```

Install `socat` through your repository's package manager if needed.
