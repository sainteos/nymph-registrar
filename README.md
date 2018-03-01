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
This can be attached to enums, classes, class methods, class method templates(with supporting arguments), static functions, and constructors.

Class method templates are a little more complicated in that you have to supply the possible types that can be used in the template within chaiscript since chaiscript needs a reference to a specific instance of the method template. This can be done as such:
```
class [[scriptable]] ClassWithTemplateMethod {
  public:

  template<typename T>
  [[scriptable(MUTATOR, {unsigned int, int, float, double, NumericType, ThreadInstance, ArbitraryType})]]
  void setArbitraryData(const T& data);
};
```
There are a couple things going on here. The first argument of the scriptable attribute "MUTATOR" lets nymph-registrar know that the method it is going to do a template substitution on is a getter/setter or some other method scheme that you would want to be named setUnsignedIntArbitraryData, setIntArbitraryData, etc. within the chaiscript runtime.

There are two other options for this: BEFORE, and AFTER for the cases in which you would want the template type name to be substituted into the function name at the beginning or at the end. What substitution scheme one uses is purely up to preference.

The next argument to the scriptable attribute is essentially an initializer list of types. Since there is only one template type for this example, it makes sense for there to be only one initializer list of types. It is also possible to do multiple template type substitution like this:
```
template<typename T, typename S, typename Z>
[[scriptable(AFTER, {unsigned int, int}, {float, double}, {std::string, const char*})]]
void doSomethingImportant(T&& t, S&& s, Z&& z);
```
When the generator creates the registration code for that template function, it will concatenate the different template types in the method name, and it will place the type names after the function name as such: `doSomethingImportantUnsignedIntFloatString`. Note how the scope drops off for the name.

### Processing A Single Library
nymph-generate runs like this:
```
nymph-registrar <options> [parse dir]
OPTIONS:
-I/path/to/include: This is an include path required to build the code in [parse dir].

-E/path/to/exclude: Use if you want to exclude specific paths from parsing.

-O/path/to/output/dir: The path in which the parse output should be placed.

-V: This tells the tool to output verbose output as it processes.

-VV: This tells the tool to output verbose output as it parses and processes.

-N: This tells the tool to generate registrations within this namespace/folder.

```
### Processing Multiple Libraries
For multiple libraries to be processed in one go, it's better to describe them within a configuration file.
```
# nymph-config.yml
---
verbose_parsing: false
verbose_processing: false
output_directory: "./generated/"
libraries:
  - name: "Library 1"
    location: "/Path/To/src/"
    includes:
      - "/Path/To/include/"
      - "/Path/To/library/include"
    namespace: "lib1"
  - name: "Library 2"
    location: "/Lib/Two/src"
    includes: "/Just/One/include"
    namespace: "lib2"
```
Running nymph-registrar with the configuration is as simple as:
```
nymph-registrar /path/to/nymph-config.yml
```

## Running the tests

There aren't tests yet. I plan to change that.

## Built With
Currently, this has been built successfully with clang. It might be possible to get it to build with gcc/g++.
* [yaml-cpp](https://github.com/jbeder/yaml-cpp) - For loading config
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
