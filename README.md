# KWC #

## About ##

`kwc` is a small 'n' fast wordcount utility. It does more or less the same job as
the UNIX `wc` utility.

## Usage ##

    kwc [options] [files]
    options:
      --lines, -l:     count lines
      --words, -w:     count words
      --chars, -c:     count character
      --parseable, -p: parseable output for use in scripts
    By default all options are enabled. If no file is specified, stdin is used.
    (C) Kurt Kanzenbach <kurt@kmk-computers.de>

## Build ##

The build process is based on autotools:

    $ ./autogen.sh
    $ ./configure --prefix=/usr
    $ sudo make install

## License ##

BSD 2-clause

## Author

(C) 2016 Kurt Kanzenbach <kurt@kmk-computers.de>
