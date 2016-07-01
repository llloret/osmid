# Build instructions
osmid uses cmake for build management.

## Requisites:
### Windows:
* cmake (version 3.2 or later)
* Microsoft Visual C++ 2015 (Community or Express editions are fine)
* Boost libraries (see http://www.boost.org/doc/libs/1_61_0/more/getting_started/windows.html). For osmid we are using the ASIO, system and program_options components. You will need to compile for 64 bits, so use `address-model=64` flag. On my machine I use the following: `b2 --build_type=minimal address-model=64`


### Linux (you can normally install all these using your distribution package manager)
* cmake (version 3.2 or later)
* gcc compiler (4.9 or later)
* Boost development libraries
* ALSA development libraries

### Mac
Don't know yet...


## Building
* go to the root directory where you have the osmid sources (probably where this file is)
* create `build` directory, and get into it (`mkdir build`, and then `cd build`)
* On Linux, it is normally enough to do `cmake ..`. If cmake complains about not finding a library make sure that it is installed.
* On Windows, you will probably need to tell cmake the path to the Boost libraries. Use `cmake .. -DBOOSTROOT=<path to boost installation>`
* On Linux, if cmake was succesful then it created the necessary Makefiles. Do `make`, and that will compile the code and create the binaries.
* On Windows, if cmake was successful then it created the necessary Visual Studio solution and Project files. Open the `osmid.sln` file, and Build normally to create the binaries.


