# Dynamic Workspaces for Sway

DISCLAIMER: Claude Code was used in the development of this tool, but all code was reviewed (and refactored) by me.

This program lets you use Sway workspaces just like Niri or GNOME, where workspaces always maintain numbering from 0 to n, and moving beyond the last workspace creates a new one (instead of looping around like Sway does by default.)

## Installation

### Nix

Flake Coming Soon!

### Other Distros

Clone this repository and run the following:

#### Automatic Installation

The following command builds the program and moves it to `/usr/local/bin`, with `sudo`.

```
make install
```

#### Manual Installation

Run the following command.

```
make
```

Move `./build/dynwork` to your PATH, or add the build directory to PATH.

## Usage

Depending on if you want to use the daemon or not, choose one of the following to add to your Sway config.

### With Daemon (Recommended)

```
exec dynwork daemon

bindgesture swipe:left exec echo left | socat - UNIX-CONNECT:$XDG_RUNTIME_DIR/dynwork.sock
bindgesture swipe:right exec echo right | socat - UNIX-CONNECT:$XDG_RUNTIME_DIR/dynwork.sock
```

Install `socat` through your distro's package manager if needed.

### Standalone

```
bindgesture swipe:left exec dynwork left
bindgesture swipe:right exec dynwork right
```

## Uninstall

If installed automatically, run the following command to uninstall.

```
make uninstall
```

## Credits

IPC code stolen from [suyjuris/i3ipc-simple](https://github.com/suyjuris/i3ipc-simple). (TYSM!)
