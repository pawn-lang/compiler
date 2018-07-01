# Pawn Compiler v3.10.x

[![Build Status][build_status]][build]
[![Build Status - Windows][build_status_win]][build_win]

## What

This is a modified copy of the Pawn compiler version 3.2.3664 by Compuphase that
fixes some bugs and adds a few features.

This project was originally maintained and managed by Zeex who left the project
due to lack of time on 2017-12-31. Thank you for all your hard work on keeping
this project alive Zeex!

The project is now maintained by the
[Compiler Team](https://github.com/orgs/pawn-lang/teams/compiler) at the
pawn-lang GitHub org.

[Original readme](readme_compuphase.txt)

## Why

The reason this exists is to:

- Fix known bugs with the original compiler
- Provide a better development experience for the SA:MP community

You shouldn't have any reason _not_ to use this compiler. If you find problem,
you can [open an issue](https://github.com/pawn-lang/compiler/issues) and
contributors can work on a fix. This isn't true of the original compiler that
comes with the SA:MP server distribution.

There are also new features that enhance the development experience, such as
telling you which tags are involved in a "tag mismatch" or showing you where
that pesky "symbol is never used" is actually declared.

There are plenty of features and fixes that are documented, see below for links:

## List of changes

- [Known compiler bugs](https://github.com/pawn-lang/compiler/wiki/Known-compiler-bugs)
  contains a list of bugs that the team are aware of with their status.

- [What's new](https://github.com/pawn-lang/compiler/wiki/What's-new) lists new
  features and other notable changes.

- [Release notes](https://github.com/pawn-lang/compiler/releases) contains a
  list of all official releases of the compiler binaries.

## How to Use

Binary packages can be downloads from
[Releases](https://github.com/pawn-lang/compiler/releases).

You can also get the latest development binaries for Windows on
[AppVeyor](https://ci.appveyor.com/project/pawn-lang/compiler/branch/master/artifacts).
This archive is built automatically on every Git commit and can be pretty
unstable, so use at your own risk.

### Windows

If you just use an editor configured to run `pawncc` such as Pawno, Sublime Text
or VS Code you can simply delete your existing `pawncc.exe` and replace it with
the new one.

Download the ZIP archive and extract `pawnc.dll`, `pawncc.exe`,
`pawndisasmsm.exe` to your original `pawncc` directory. If you use Pawno, this
will be in the `pawno/` folder that's inside your server directory.

### openSUSE/SLES

There is an installation package available for openSUSE/SLES users so that you
can easily install the compiler on your distribution. Please follow these steps:

1.  Go to https://build.opensuse.org/package/show/home:mschnitzer/pawncc
2.  On the right side, select your distribution (only if it's not disabled!)
3.  Click "Go to download repository"
4.  Copy the link and enter in sudo mode in your shell:
    `zypper ar $COPIED_LINK home:mschnitzer`
5.  Again as root, type: `zypper ref`
6.  Install the package with `zypper in pawncc`
7.  Run `pawncc` in your shell to test if it's working

### With sampctl

If you are a sampctl user, simply set the `version` field in `build`/`builds`:

```json
{
  "builds": [
    {
      "name": "production",
      "version": "3.10.8"
    }
  ]
}
```

## Building from source code

In general you will need [CMake](https://cmake.org/) and a C compiller to build
Pawn from source code.

### Building on Windows

- Clone this repo: `git clone https://github.com/Zeex/pawn.git C:\pawn` (you can
  use another directory instead of `C:\Pawn`, but make sure the path doesn't
  have spaces).
- Install [Visual Studio Community](https://www.visualstudio.com/vs/community/),
  it's free.
- Install [CMake](https://cmake.org/).

  When installing make sure to check "Add CMake to system PATH" to make your
  life easier.

- Generate a Visual Studio project.

  In Command promprt or Powershell execute the following:

  ```cmd
  cd C:\Pawn
  mkdir build && cd build
  cmake ..\source\compiler -G "Visual Studio 15 2017"
  ```

- From the same directory as in the previous step run:

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

`gcc-multilib` is needed for compiling a 32-bit binary (64-bit is not
supported).

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

- Install Xcode: https://developer.apple.com/xcode/

- Install Command Line Tools for Xcode:

```sh
xcode-select --install
```

- Install CMake:

```sh
brew install cmake
```

- Now you can clone this repo and build the compiler:

```sh
git clone https://github.com/Zeex/pawn.git ~/pawn
cd ~/pawn
mkdir build && cd build
cmake ../source/compiler -DCMAKE_C_FLAGS=-m32 -DCMAKE_BUILD_TYPE=Release
make
```

## Background

The project was originally started as a set of patches aimed to create a
compiler that would be compatible with the compiler used in
[SA-MP (San Andreas Multiplayer)](http://sa-mp.com/).

SA-MP uses a modified version of Pawn 3.2.3664 [1] with Windows-only
executables, and the developers said that they lost the source code for it which
means it can't be ported to other platforms (e.g. Linux) and newly discovered
bugs can't be fixed. So the main goal of the project is to re-create changes
that were previously made by the devs as well as fix all known compiler bugs.

[1] It's worth noting that the version of the AMX embedded into the SA-MP server
seems to be based on an older release of Pawn.

[build]: https://travis-ci.org/pawn-lang/compiler
[build_status]: https://travis-ci.org/pawn-lang/compiler.svg?branch=master
[build_win]: https://ci.appveyor.com/project/Southclaws/compiler/branch/master
[build_status_win]:
  https://ci.appveyor.com/api/projects/status/k112tbr1afrkif0n?svg=true
