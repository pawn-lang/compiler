Pawn
====

[![Build Status][build_status]][build]
[![Build Status - Windows][build_status_win]][build_win]
[![Gitter][gitter_badge]][gitter]

[Original readme](readme_compuphase.txt)

**This project is no longer maintained. Because of the lack of time and interest I can't add new fixes and features or accept pull requests. But feel free to create a fork to continue development. Thank you!**

What is this?
-------------

This is a modified copy of the Pawn compiler version 3.2.3664 by Compuphase that
fixes some bugs and adds a few features.

List of changes
---------------

See [Known compiler bugs](../../wiki/Known-compiler-bugs) for the list of fixed
bugs and [What's new](../../wiki/What's-new) for the list of features and other
notable changes.

Installation on openSUSE/SLES
-----------------------------

There is an installation package available for openSUSE/SLES users so that you can
easily install the compiler on your distribution. Please follow these steps:

1. Go to https://build.opensuse.org/package/show/home:mschnitzer/pawncc
2. On the right side, select your distribution (only if it's not disabled!)
3. Click "Go to download repository"
4. Copy the link and enter in sudo mode in your shell: `zypper ar $COPIED_LINK home:mschnitzer`
5. Again as root, type: `zypper ref`
6. Install the package with `zypper in pawncc`
7. Run `pawncc` in your shell to test if it's working

Building from source code
-------------------------

In general you will need [CMake](https://cmake.org/) and a C compiller to build
Pawn from source code.

### Building on Windows

* Clone this repo: `git clone https://github.com/Zeex/pawn.git C:\pawn`  (you can
  use another directory instead of `C:\Pawn`, but make sure the path doesn't have
  spaces).
* Install [Visual Studio Community](https://www.visualstudio.com/vs/community/),
  it's free.
* Install [CMake](https://cmake.org/).

  When installing make sure to check "Add CMake to system PATH" to make your life
  easier.
  
* Generate a Visual Studio project.

  In Command promprt or Powershell execute the following:
  
  ```cmd
  cd C:\Pawn
  mkdir build && cd build
  cmake ..\source\compiler -G "Visual Studio 15 2017"
  ```
  
* From the same directory as in the previous step run:

  ```
  cmake --build . --config Release
  ```
  
  or open the pawnc.sln in Visual Studio and build from there (but make sure to
  choose the "Release" configuration).

  This will create `pawnc.dll` and `pawncc.exe` in the `Release` folder. You can
  now copy these files to your `pawno` folder for convenience or put them in a
  separate folder and configure your code editor accordingly.

### Building on Linux

Use your distribution's package manager to install the required dependencies.
For example, in Ubuntu you would do:

```sh
sudo apt install gcc gcc-multilib make cmake
```

`gcc-multilib` is needed for compiling a 32-bit binary (64-bit is not supported).

Now you can clone this repo and build the compiler:

```sh
git clone https://github.com/Zeex/pawn.git ~/pawn
cd ~/pawn
mkdir build && cd build
cmake ../source/compiler -DCMAKE_C_FLAGS=-m32 -DCMAKE_BUILD_TYPE=Release
make
```

Replace "Release" with "Debug" if you want to build a debug executable for
development or submitting bugs.

### Building on macOS

* Install Xcode: https://developer.apple.com/xcode/

* Install Command Line Tools for Xcode:

```sh
xcode-select --install
```

* Install CMake:

```sh
brew install cmake
```

* Now you can clone this repo and build the compiler:

```sh
git clone https://github.com/Zeex/pawn.git ~/pawn
cd ~/pawn
mkdir build && cd build
cmake ../source/compiler -DCMAKE_C_FLAGS=-m32 -DCMAKE_BUILD_TYPE=Release
make
```

Background
----------

The project was originally started as a set of patches aimed to create a compiler
that would be compatible with the compiler used in [SA-MP (San Andreas Multiplayer)](http://sa-mp.com/).

SA-MP uses a modified version of Pawn 3.2.3664 [1] with Windows-only executables,
and the developers said that they lost the source code for it which means it can't
be ported to other platforms (e.g. Linux) and newly discovered bugs can't be fixed.
So the main goal of the project is to re-create changes that were
previously made by the devs as well as fix all known compiler bugs.

[1] It's worth noting that the version of the AMX embedded into the SA-MP server
seems to be based on an older release of Pawn.

[build]: https://travis-ci.org/Zeex/pawn
[build_status]: https://travis-ci.org/Zeex/pawn.svg?branch=master
[build_win]: https://ci.appveyor.com/project/Zeex/pawn/branch/master
[build_status_win]: https://ci.appveyor.com/api/projects/status/s1gb9p8dsy7hy1nw?svg=true
[gitter]: https://gitter.im/Zeex/pawn?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge
[gitter_badge]: https://badges.gitter.im/Join%20Chat.svg
