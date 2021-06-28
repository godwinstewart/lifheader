# lifheader

** Copyright (c) 2021 Godwin Stewart <g.stewart*@*horwits.co.uk>**

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

