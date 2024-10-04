# How to install

The command-line version executable file compiled by this project distinguishes whether to run in server mode or client mode according to the parameters, so the installation does not distinguish between client and server.

## Linux

Due to the differences in Linux environments, multiple installation methods are provided. Please choose the method that suits you.

### Docker

It is recommended to use the Docker image, which has been uploaded to [Docker Hub](https://hub.docker.com/r/lanthora/candy) and [Github Packages](https://github.com/lanthora/candy/pkgs/container/candy).

The container requires administrator privileges to read the device to create a virtual network card and set up routing, and the host network namespace needs to share the virtual network card.

```bash
docker run --rm --privileged=true --net=host docker.io/lanthora/candy:latest --help
```

### Arch Linux

Use [AUR](https://aur.archlinux.org/packages/candy) or [archlinuxcn](https://github.com/archlinuxcn/repo/tree/master/archlinuxcn/candy) repository

```bash
# AUR
paru candy
# archlinuxcn
pacman -S candy
```

### Gentoo

```bash
emerge --sync gentoo && emerge -av candy
```

### openSUSE

Use [OBS](https://software.opensuse.org/download/package?package=candy&project=home:lanthora:candy) repository

```bash
# Take Tumbleweed as an example, choose to trust the signature when updating the repository cache
zypper addrepo https://download.opensuse.org/repositories/home:lanthora:candy/openSUSE_Tumbleweed/home:lanthora:candy.repo
zypper refresh && zypper install candy
```

### Debian/Ubuntu

Download the [DEB](https://github.com/lanthora/candy/releases/latest) package and install it with the following command

```bash
apt install --fix-broken ./xxx.deb
```

### Single-file executable program

When all the above methods are not applicable, try [single-file executable program](https://github.com/lanthora/candy/releases/latest).

This program is built by [cross-compilation script](https://github.com/lanthora/candy/tree/master/scripts/build-standalone.sh).

## macOS

Please refer to the method provided in [Homebrew](https://github.com/lanthora/homebrew-repo) for installation.

## Windows

### Graphical User Interface

Build [Cake](https://github.com/lanthora/cake) with this project as a dependency Provide [Graphical User Interface](https://github.com/lanthora/cake/releases/latest).

### Command Line

This project only provides [Command Line Version](https://github.com/lanthora/candy/releases/latest), users can customize it based on this.

## Build from Source Code

### Build Native Single-File Executable Program

Compiler dependent on `C++20`.

```bash
cmake -B build -DCANDY_STATIC=1
cmake --build build
cmake --install build
```

### Cross-compile Linux Single-File Executable Program

Set the following environment variables according to the actual situation, check [Supported Systems and Architectures](https://github.com/lanthora/candy/tree/master/scripts/standalone.json).

```bash
# Absolute path to the directory used for downloading and compiling
export CANDY_WORKSPACE=$HOME/workspace
# Operating System
export CANDY_OS=linux
# Target file architecture
export CANDY_ARCH=x86_64
```

Execute the build script. The compilation toolchain and dependent libraries will be downloaded during the build. Please ensure that the network is unobstructed.

```bash
scripts/build-standalone.sh
```

The generated binary file is `$CANDY_WORKSPACE/output/$CANDY_OS-$CANDY_ARCH/candy`.
