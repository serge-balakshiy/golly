# Brief description of the project

This is a research project. The main research issue is the introduction of the forth system into applications written in C, C ++, so that the script written in the forth can be called directly from the application and the resources of the application itself can be used in this script.

# Objective of the project

Explore ways to embed a forth into applications.

# Tasks include

1. Research and description of ways to implement the Forth in applications.
2. development of the interface for implementation. What properties should the system have to allow you to do this easily and quickly?

# Who the project is intended for

For those researchers and developers who are interested in the Fort.

### Build
You will need the wxWidget package.

To use a `Git` mirror, use one of the following commands

git clone --recursive https://github.com/serge-balakshiy/golly.git

cd golly/gui-wx/configure

./autogen.sh

./configure

make

#### Run Golly

./golly

#### Run Script

Menu File/Run Script
select directory Scripts/CxxForth
select file tdl2-04.0-02.cxxfs
Click to start drawing a line. Draw a line. Click to finish drawing the line.
And also part of the script is executed on atlast. This is Scripts/Atlast/tdl-00.01.atl.
These scripts are based on the lua script. This is Scripts/Lua/draw-lines.lua.

Now I want to do the same for gforth. 
But so far I have not succeeded. The problem is that I cannot implement the necessary interface functions within the golly framework. The gforth / unix / example is good for a simple project. I need to do something similar in golly. I could also do it in librecad or some other program.

For this project. 
In the gui-wx directory were created.
To run CxxForth in golly I created a files wxcxxforth.cpp and wxcxxforth.h
To run ATLAST forth in golly I created a files wxatlast.cpp and wxatlast.h

To run gforth in golly I created a wxgforth.cpp and wxgforth.h files.

Lua scripts are described [here](http://golly.sourceforge.net/Help/lua.html)

Other scripts have the same interface. Lua, Python, Perl.
I have implemented part of this interface in CxxForth and ATLAST forth.
And I want to implement this in gforth.
