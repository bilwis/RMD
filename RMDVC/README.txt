ROGUE, M.D. pre-alpha
Copyright 2014 bilwis.de

I. Introduction

This will hopefully one day be a proper roguelike. For now, the focus is on the development of the 
underlying systems simulating body structure/parts and "pseudo-medical" interactions between these systems.

II. Build instructions

a) Dependencies/Libraries included in repository
libtcod-v.1.5.1-MSVC [http://doryen.eptalys.net/libtcod/]
	zlib 1.2.3
	lodepng 20120729
	upx
SDL 1.2.12 [https://www.libsdl.org/]

b) Dependencies NOT included in repository (these must be added to the /lib folder in order to compile with the .sln settings)
boost_1_56_0 [http://www.boost.org/]


c) Additional Software required for Documentation
DoxyGen (use RMDVC.doxyfile for config)
Graphviz (for DoxyGen and parsing the Body Map the Debug config creates during runtime [body.gv])

III. Licenses

see README-SDL and LIBTCOD-LICENCE