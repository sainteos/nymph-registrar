# nymph-registrar

nymph-registrar is a replacement for nymph-generate. nymph-generate was the first attempt at having an automated way to parse libraries for use within chaiscript. It was built as a rubygem and used regex parsing to search header files. This worked pretty well until I wanted to use it with different libraries, including ones not written by me. That brought me to leveraging libclang by the use of cppast to parse the C++ abstract syntax tree and generate chaiscript module registrations using that. This turned out to be a much better way to do so.

## Building

To get nymph-registrar, just clone it like any other repository.
```
git clone https://github.com/sainteos/nymph-registrar.git
cd nymph-registrar
```
Then it is a matter of running premake and make.
```
premake4 gmake
make
```
The output will be a binary called `nymph-registrar`.

### Prerequisites

nymph-registrar requires [libclang](https://github.com/llvm-mirror/clang) within the clang project and [cppast](https://github.com/foonathan/cppast) to build.

Cppast is included within ext/ as a git submodule. You can pull it with:
```
git submodule init
git submodule --recursive update
```
Build cppast as per its README, and then create a symlink to its libraries in the lib/ folder.
```
mkdir lib
ln ext/cppast/src/libcppast.a lib/cppast.a
ln ext/cppast/lib_cppast_tiny_process.a lib/lib_cppast_tiny_process.a
```

### Installing

Either move the binary somewhere that is on your PATH, or add the directory in which the binary resides to your PATH.

## Running
Mark up the header files you would like to parse with attributes that look like this.
```
[[scriptable]]
```
This can be attached to enums, classes, class methods, static functions, and constructors.
nymph-generate runs like this:
```
nymph-registrar <options> [parse dir]
OPTIONS:
-I/path/to/include: This is an include path required to build the code in [parse dir].

-E/path/to/exclude: Use if you want to exclude specific paths from parsing.

-O/path/to/output/dir: The path in which the parse output should be placed.

-V: This tells the tool to output a lot more information.

-X: This tells the tool to expand the generated registrations into multiple files per namespace.

-S: This tells the tool to output the generated registrations to stdout.
```
## Running the tests

There aren't tests yet. I plan to change that.

## Built With
Currently, this has been built successfully with clang. It might be possible to get it to build with gcc/g++.
* [cppast](https://github.com/foonathan/cppast) - For searching the abstract source tree
* [libclang](https://github.com/llvm-mirror/clang) - Used by cppast to read the source tree.
* [premake4](https://github.com/premake/premake-4.x) - For generating a makefile

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/your/project/tags).

## Authors

* **Adaleigh Martin** -- Transferred from nymph-generate

## License

I am unsure about licensing at this point.

## Acknowledgments

* Thanks to foonathan, cppast was exactly what I needed.
