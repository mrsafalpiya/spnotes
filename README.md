# spnotes - Safal Piya's Notes

## About

`spnotes` is a simple note taking single-file public domain library for C.

## As a library

This library is responsible only for getting the list of categories and notes
listing the title and description. Any other note related implementations such
as creating a category or a note entry is upto the user.

This makes the library rather extensible: one can use shell script to handle
creation and modification of categories/notes meanwhile a C program handles the
heavy lifting of listing them.

### Usage

Simple copy the `dep/spnotes.h` file into your project and do this:

```c
#define SPNOTES_IMPL
```
before you include this file in *one* C or C++ file to create the
implementation.

### Dependencies

- Computer with a C99 compliant C compiler.
- `#define _POSIX_C_SOURCE 200809L` (for strdup() and strndup())
- `#define _DEFAULT_SOURCE` (for d_type macro constants)

## As a standalone program

Users are supposed to use this library to create their own implementation of
`spnotes`. For reference, a fully functional program (that I personally use) is
provided in this repo as `spnotes-cli.c`.

### Installing the demo program

Simply run

```sh
make release
```
to compile the program and

```sh
make install
```
(requires root privilege) to install the program to the system.

## Note taking system layout

The note structure of spnotes is described below:

Basic tree structure:

```
$ pwd
/home/safal/notes
$ tree
.
├── binary-digits
│   └── 1648221020.md
├── c
│   ├── 1648296557.md
│   └── 1648362649.md
└── latex
    ├── 1650081313.md
    └── 1650081524.md

3 directories, 5 files
```

Here, 'binary-digits', 'c' and 'latex' are so called `categories` of the system
and each md file inside represents an individual note within the category. Any
file starting with a '.' is ignored. Only those files ending with a '.md' is
regarded as a note.

All notes should have the following structure:

```
$ cat c/1648362649.md
---
title: Pipes
description: About pipes in C
---

# Bi-directional pipe

src: https://youtu.be/8AXEHrQTf3I
$ cat c/1648296557.md
---
title: `static inline` keyword
description: Usage of `static inline` keyword in C
---

# The actual use of `static inline`

Ref: https://youtu.be/sJuA5OPvABM
```

Note the first line of the file starting with `---` following a title,
description and ending with `---`. The description is optional but other
components has to be present in the file to be regarded as a note.

## Tested platforms

I have successfully compiled this library under following platforms:

- GNU/Linux - Arch linux
- OpenBSD
- Windows with Cygwin
