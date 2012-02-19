To generate project files for your primary compiler, run:

cmake .

The lib files in libmodplug/ were built with Visual Studio 2010 (Release),
you might want to replace them or explicitly generate a VS2010 project:

cmake -G "Visual Studio 10"

To build the sample as a 64-bit binary, use

cmake -G "Visual Studio 10 Win64"
