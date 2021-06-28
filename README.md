# lifheader

This is a utility designed to work in tandem with Tony Duell's original [lifutils](https://github.com/bug400/lifutils).

`lifheader` can display the information contained in the LIF header of a file
extracted from a LIF disk image, it can strip said header from the file and it
can build a new LIF header to prepend to a file that doesn't yet have one, thus
preparing it for inclusion in a LIF disk image using the `lifput` tool that is
part of `lifutils`.
