# Contributing to the Pawn Compiler

If you are proposing a new feature or fixing a bug of significant complexity, please do not write any code and propose your planned changes via an issue first! This saves _you_ time in case changes are required or the feature is rejected and it saves the _maintainers_ time as discussing proposals is generally easier than doing code review. Thank you!

## Pull Requests

### Branches

This repository follows a simple feature branch with a dev branch for staging. `master` should always reflect the latest release and `dev` is the staging branch for the next version. All other features must be worked on in feature branches.

### Pull Contents

Ensure you only commit source files, test resources or other portable assets. Pulls that contain editor configs, executables, libraries and other non-source files will be rejected unless there's a good reason.

## Issues

### Questions

Feel free to open issues that are just questions, they will be marked as a question and answered by a maintainer or community member.

However, do not ask about compiler errors and warnings that are the fault of Pawn code. This repository is purely for discussing the compiler itself, it is not a Pawn help channel. That being said, if you have come across an error or warning you did not expect from the given code, open an issue and we can investigate if it's a bug in the compiler.

### Reproductions

Always submit code to reproduce a bug, if your gamemode code is private then it's up to you to create a minimal reproduction case.
