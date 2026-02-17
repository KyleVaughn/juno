# env

This repo exists to build the tools we would like to have in a developer environment for
use with neovim and its plugins.

## Getting started

Source the `env.sh` script to load the developer environment. This will load the
modules needed to make neovim and the associated tools available.

## What is built?

- `fd` is a command-line utility written in Rust that serves as a modern, user-friendly,
and faster alternative to the traditional Unix `find` command.

- `llvm` provides `clangd`, `clang-format`, and `clang-tidy`---a C++ language server, a
    formatter, and a static analysis tool, respectively

- `neovim` is a modern, open-source text editor that is a fork of the original Vim editor,
built to be more extensible, usable, and maintainable.

- `py-fortls` is a fortran language server

- `ripgrep` (`rg`) is a command-line search tool that recursively searches for patterns in files, known
for its speed and ability to respect `.gitignore` rules by default. Essentially, it is a
better `grep`.
