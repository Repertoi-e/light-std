These get added by the premake file that handles `lstd/`, 
i.e. they should be part of the library and not a separate compilation.

In the main premake5.lua file you should set, for example

LSTD_INCLUDE_EXTRAS = { "guid", "signal" }

to include the files in the listed directories.
