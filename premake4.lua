#!lua
solution "nymph-registrar"
  configurations { "DEBUG" }

  project "nymph-registrar"
    kind "ConsoleApp"
    language "C++"

    targetname "nymph-registrar"

    includedirs { "./include", "./ext/cppast/external/type_safe/include", "./ext/cppast/external/type_safe/external/debug_assert", "./src" }
    libdirs { "./lib", "/usr/local/opt/llvm/lib" }
    excludes { "./ext/**", "./test/**" }

    files { "**.h", "**.hpp", "**.cpp", "**.cc" }

    configuration "DEBUG"
      buildoptions { "-std=c++14", "-g", "-O0", "-Wno-assume" }
      links {  "yaml-cpp", "cppast", "_cppast_tiny_process", "clang" }
      defines { "DEBUG" }
      flags { "Symbols" }
