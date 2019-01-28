# OpenSlay

OpenSlay is a cross platform, open source, and free remake of the game [Slay by Sean O'Connor](http://www.windowsgames.co.uk/slay.html).

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

* **Build tools:**
  * C compiler
  * Bam
* **Libraries**
  * SDL2
  * SDL2_image



### Build Setup

You will problably need to install Bam manualy as it isn't quite popular. You can download it from https://github.com/matricks/bam. It should have an easy to use script file to build itself. You will need a C Compiler.

For Debian libaries:
```
sudo apt-get install build-essential libsdl2-dev libsdl2-image-dev
```

**Windows**
If you are building in windows you need to have access to the visual studio compiler tools in order to build. To get that, you can run the SetupWindowsCompiler.bat file

```
SetupWindowsCompiler.bat
```

### Building


All you need to do is run Bam from the root directory.

```
bam
```

### Installing

Currently you just run OpenSlay as it is. There is not support for installing.


### Running

After you built the project you can run OpenSlay from the root directory.

```
./openslay
```

## Running the tests

There are currently no tests for OpenSlay.

## Authors

* **Ryan Hanson**

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Acknowledgments

Specail thanks to Sean O'Connor for inventing the game. It is designed really well.


