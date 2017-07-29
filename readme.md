Pawn
====

[![Build Status][build_status]][build]
[![Build Status - Windows][build_status_win]][build_win]
[![Gitter][gitter_badge]][gitter]

[Original readme](readme.txt)

What is this?
-------------

This is a modified copy of the Pawn compiler version 3.2.3664 by Compuphase that
fixes some bugs and adds a few features.

Installation on openSUSE/SLES
---------------------
There is an installation package available for openSUSE/SLES users so that you can easily install the compiler on your distribution. Please follow these steps:

1. Go to https://build.opensuse.org/package/show/home:mschnitzer/pawncc
2. On the right side, select your distribution (only if it's not disabled!)
3. Click "Go to download repository"
4. Copy the link and enter in sudo mode in your shell: `zypper ar $COPIED_LINK home:mschnitzer`
5. Again as root, type: `zypper ref`
6. Install the package with `zypper in pawncc`
7. Run `pawncc` in your shell to test if it's working

Changes
-------

See [Known compiler bugs](../../wiki/Known-compiler-bugs) for the list of fixed
bugs and [What's new](../../wiki/What's-new) for the list of features and other
notable changes.

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
