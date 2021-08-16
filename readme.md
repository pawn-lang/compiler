# Pawn Community Compiler

[![Build Status][build_status_linux]][build_linux]
[![Build Status - Windows][build_status_win]][build_win]

## What

This is a modified version of the Pawn 3.2.3664 compiler with many bug fixes and
enhancements.

This project was originally started by Zeex but on 2017-12-31, the project was
taken over by some members of the SA:MP community. Zeex still contributes to the
project, along with the [Compiler Team][team].

The original readme is available [here][original_readme]

## Why

This project exists to:

- Fix known bugs with the original compiler
- Provide a better development experience for the SA:MP community

If you find a problem, you can [open an issue][issues] and contributors can work
on a fix. This isn't true with the original compiler that comes with the SA:MP
server distribution.

There are also new features that enhance the development experience, such as
telling you which tags are involved in a "tag mismatch" or showing you where
that pesky "symbol is never used" is actually declared.

There are plenty of features and fixes that are documented, see below for links:

- [Known compiler bugs][bugs] contains a list of bugs that the team are aware of
  with their status.

- [What's new][new] contains features and other notable changes.

- [Release notes][releases] list of all official releases of the compiler
  binaries.

## How to Use

Binary packages can be downloaded from [Releases][releases], see the below
sections for platform-specific installation instructions.

**Note:** You will _probably_ get warnings/errors/fatals when you first build
with this compiler — you need to add the `-Z` flag to your build configuration
or add `#pragma compat`. See [this page][compat] for more information.

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

### openSUSE

There is an installation package available for openSUSE users so that you
can easily install the latest compiler on your distribution. Please follow
these steps:

1.  Go to https://build.opensuse.org/package/show/home:mschnitzer:pawncc/pawncc
2.  On the right side, select your distribution (only if it's not disabled!)
3.  Click "Go to download repository"
4.  Copy the link and enter in sudo mode in your shell:
    `zypper ar $COPIED_LINK home:mschnitzer:pawncc`
5.  Again as root, type: `zypper ref`
6.  Install the package with `zypper in pawncc`
7.  Run `pawncc` in your shell to test if it's working

The download repository for openSUSE does provide older versions as well (oldest
version: 3.10.7). Just install the version you like (e.g.:
`zypper in pawncc-3.10.8` and run it via `$ pawncc-3.10.8 -Z -O3 [...]`).
Parallel installation is supported.

### With sampctl

If you are a [sampctl][sampctl] user, you are already using this compiler!

### Building from Source

If you are interested in contributing then please first read
[this document][contributing] and ensure you have discussed your proposed
changes before writing any code. Check out [this page][build_source] for
instructions for compiling for your platform.

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
[team]: https://github.com/pawn-lang/compiler/graphs/contributors
[original_readme]:
  https://github.com/pawn-lang/compiler/tree/master/readme_compuphase.txt
[issues]: https://github.com/pawn-lang/compiler/issues
[bugs]: https://github.com/pawn-lang/compiler/wiki/Known-compiler-bugs
[new]: https://github.com/pawn-lang/compiler/wiki/What's-new
[releases]: https://github.com/pawn-lang/compiler/releases
[artifacts]:
  https://ci.appveyor.com/project/Southclaws/compiler/branch/master/artifacts
[compat]: https://github.com/pawn-lang/compiler/wiki/Compatibility-mode
[sampctl]: https://github.com/Southclaws/sampctl
[contributing]:
  https://github.com/pawn-lang/compiler/tree/master/.github/CONTRIBUTING.md
[build_source]: https://github.com/pawn-lang/compiler/wiki/Building-From-Source
