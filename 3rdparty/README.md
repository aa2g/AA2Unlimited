# 3rd party compiling gotchas

These are in the proud tradition of vcxproj horrors. Meaning you need to open
.sln file for each:

* zstd/build/VS2010/zstd.sln
* opus/win32/VS2015/opus.sln

Open in new workspace - do NOT import into AA2U, it's futile and the individual
vcxprojs are not portable. Just select static library Release/x86 target
for each, build close, and forget this ever happened.

AA2U in parent directory will then see these .lib:

* opus/win32/VS2015/Win32/Release/opus.lib
* zstd/build/VS2010/bin/Win32\_Release/libzstd\_static.lib

Yes I wish there was a better way, but microsoft has yet to invent something
even vaguely resembling a makefile.
