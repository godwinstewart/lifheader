# lifheader

**Copyright (c) 2021 Godwin Stewart**

This is a utility designed to work in tandem with Tony Duell's original
[lifutils](https://github.com/bug400/lifutils). It is based both on `lifutils`
and on Jean-François Garnier's `alifhdr` utility.

`lifheader` can display the information contained in the LIF header of a file
extracted from a LIF disk image, it can strip said header from the file and it
can build a new LIF header to prepend to a file that doesn't yet have one, thus
preparing it for inclusion in a LIF disk image using the `lifput` tool that is
part of `lifutils`.

`lifheader` has been built and works on a Linux system. It should probably build and
run with little to no modification on MacOS. It'll probably require some tweaks to
build and run on Win32.

## Usage
```
        lifheader { -a action | -h } [ -i input_file ] [ -o output_file ] [ -t file_type ]
                  [ -l lif_file_name ]

        -h                Shows this help message.

        -a action         Specifies the action to undertake on the input file. Possible options are:
                -a strip        Strips the LIF header from the input file.
                -a add          Generates a LIF header, prepends it to the input file
                                and saves the result to the output file
                -a show         Shows the data in the LIF header.

        -i input_file     Designates the input file to read from. If not given
                          or if the string `-' is given, then STDIN is used.

        -o output_file    Designates the output file to write to. If not given
                          or if the string `-' is given, then STDOUT is used.

        -t file_type      When adding a LIF header to a file, specifies the file type
                          to indicate in the header. Possible options are:
                -t lex71        HP-71B LEX file (0xe208)
                -t bas71        HP-71B BASIC file (0xe214)
                -t rom71        HP-71B ROM file (0xe21c)
                -t key71        HP-71B key assignments (0xe20c)
                -t txt71        HP-71B text file (0x0001)
                -t bin71        HP-71B BIN file (0xe204)
                -t prg41        HP-41C program (0xe080)
                -t sdata        HP-71B SDATA/HP-41C data file (0xe0d0)
                -t key41        HP-41C key assignments (0xe050)
                -t sta41        HP-41C status file (0xe060)
                -t all41        HP-41C "WALL" file (0xe040)
                -t rom41        HP-41C ROM/MLDL dump (0xe070)

        -l lif_file_name  Provides the name for the file in the LIF image when adding a
                          LIF header to a file. The name is deduced from the original
                          filename if not given on the command line.
```
