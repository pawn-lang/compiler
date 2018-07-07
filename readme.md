# Pawn Community Compiler

[![Build Status][build_status_linux]][build_linux]
[![Build Status - Windows][build_status_win]][build_win]

## What

This is a modified version of the Pawn 3.2.3664 compiler with many bug fixes and
enhancements.

This project was originally founded by Zeex but on 2017-12-31, the project was
taken over by some members of the SA:MP community. Zeex still contributes to the
project, along with the [Compiler Team][team].

The original readme is available [here][original_readme]

## Why

This project exists to:

- Fix known bugs with the original compiler
- Provide a better development experience for the SA:MP community

If you find problem, you can [open an issue][issues] and contributors can work
on a fix. This isn't true of the original compiler that comes with the SA:MP
server distribution.

There are also new features that enhance the development experience, such as
telling you which tags are involved in a "tag mismatch" or showing you where
that pesky "symbol is never used" is actually declared.

There are plenty of features and fixes that are documented, see below for links:

## List of changes

- [Known compiler bugs][bugs] contains a list of bugs that the team are aware of
  with their status.

- [What's new][new] contains features and other notable changes.

- [Release notes][releases] list of all official releases of the compiler
  binaries.

## How to Use

Binary packages can be downloaded from [Releases][releases].

You can also get the latest development binaries for Windows on
[AppVeyor][artifacts]. This archive is built automatically on every Git commit
and can be pretty unstable, so use at your own risk.

### Windows

If you just use an editor configured to run `pawncc` such as Pawno, Sublime Text
or VS Code you can simply delete your existing `pawncc.exe` and replace it with
the new one.

Download the ZIP archive and extract `pawnc.dll`, `pawncc.exe`,
`pawndisasmsm.exe` to your original `pawncc` directory. If you use Pawno, this
will be in the `pawno/` folder that's inside your server directory.

You must add the `-Z+` flag to your build configuration or add `#pragma compat`
to your script. See [this page][compat] for more information.

### openSUSE/SLES

There is an installation package available for openSUSE/SLES users so that you
can easily install the compiler on your distribution. Please follow these steps:

1.  Go to <https://build.opensuse.org/package/show/home:mschnitzer/pawncc>
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

If you have VS 2017 or later:

- Clone this repo: `git clone https://github.com/Zeex/pawn.git`
- In VS 2017: File > Open > CMake > Select the `CMakeLists.txt` file in the root
  of the compiler repository.

Otherwise:

- Install [Visual Studio Community](https://www.visualstudio.com/vs/community/)
- Clone this repo: `git clone https://github.com/Zeex/pawn.git`
- Install [CMake](https://cmake.org/). When installing make sure to check "Add
  CMake to system PATH" to make your life easier.

- Generate a Visual Studio project. In Command promprt or Powershell execute the
  following:

  ```cmd
  cd C:\Pawn
  mkdir build && cd build
  cmake ..\source\compiler -G "Visual Studio 15 2017"
  ```

- From the same directory as in the previous step run:

  ```cmd
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

[build_linux]: https://travis-ci.org/pawn-lang/compiler
[build_status_linux]: https://travis-ci.org/pawn-lang/compiler.svg?branch=master
[build_win]: https://ci.appveyor.com/project/Southclaws/compiler/branch/master
[build_status_win]:
  https://ci.appveyor.com/api/projects/status/k112tbr1afrkif0n?svg=true
[team]: https://github.com/orgs/pawn-lang/teams/compiler
[original_readme]:
  https://github.com/pawn-lang/tree/master/readme_compuphase.txt
[issues]: https://github.com/pawn-lang/compiler/issues
[bugs]: https://github.com/pawn-lang/compiler/wiki/Known-compiler-bugs
[new]: https://github.com/pawn-lang/compiler/wiki/What's-new
[releases]: https://github.com/pawn-lang/compiler/releases
[artifacts]:
  https://ci.appveyor.com/project/Southclaws/compiler/branch/master/artifacts
[compat]: https://github.com/pawn-lang/compiler/wiki/Compatibility-mode
