# KWC #

## About ##

`kwc` is a small 'n' fast wordcount utility. It does more or less the same job as
the UNIX `wc` utility.

## Usage ##

    kwc [options] [file]
    options:
      -l: count lines
      -w: count words
      -b: count bytes
      -p: parseable output for use in scripts
    By default all options are enabled. If no file is specified, stdin is used.
    (C) Kurt Kanzenbach <kurt@kmk-computers.de>

## Build ##

The build process uses a simple Makefile which uses GNU features. Thus, you have
to use GNU Make for building it.

On Linux use:

    $ make

On UNIX systems use:

    $ gmake

By default the Makefile uses clang as compiler, but you can override some
variables by hand. Example for gcc:

    $ CC=gcc5 make

## License ##

BSD 2-clause

## Author

(C) 2016 Kurt Kanzenbach <kurt@kmk-computers.de>
