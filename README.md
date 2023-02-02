# XNFSTPKTool

A general purpose TexturePack tool for NFS games

## Game compatibility

This tool should now be compatible with almost every NFS game built on EAGL/Black Box NFS.

## Platforms

Technically every from one perspective as it can extract data from any TPK.
However, it produces working files from/to the following platforms so far:

- PC
- PS3
- Xbox 360 (Giant work in progress)

## Building

The included solution is made from VS2015, however, you should be able to build this with any Visual Studio or Dev C++.

- Open XNFSTPKTool.sln
- Select the configuration that you desire
- Output will be in the configuration's folder in the root directory

# TODO

- A lot of cleanup. String operations are attrocious, beware of wolves in the code!
- Multiple decompression types - only tested with PRECOMPVINYLS so far, seems to have worked!
- Code refactoring and splitting
- TPK version autodetection
- More platforms
