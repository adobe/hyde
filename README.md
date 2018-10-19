# What is hyde

The primary goal of hyde for the photoshop team was a way to generate documentation from our current C++ source code, in such a way that can be consumed by jekyll to create static standalone documentation site. The project was born out of what we felt was a limitation or difference of opinion in the approach existing documention tooling. That often requires you to litter your source with inline comments that detracts from readability. These comments also tend to become out of date over time unless you are hyper-vigilant. Hyde can detect this and be used to prevent merging code that doesn't update the documentation.

Whilst the project was built around this use case, care has been taken to create a tool that is adaptable. You can build more tooling on top of the json format that we turn the clang ast into, or you can simply write your own emitter to massage the output into something that will suit your needs.


# Requirements

- Homebrew
    - `brew install cmake`
    - `brew install llvm`
    - `brew install yaml-cpp`
    - `brew install boost`
    - `brew install ninja` (optional)

# How to Build

- clone this repo
- `cd hyde`
- `mkdir build`
- `cd build`
- `cmake .. -GNinja` (or `-GXcode`, etc.)
- `ninja` (or whatever your IDE does)

# Parameters and Flags

There are several modes under which the tool can run:

- `-hyde-json` - (default) Output an analysis dump of the input file as JSON
- `-hyde-validate` - Validate existing YAML documentation
- `-hyde-update` - Write updated YAML documentation

- `-hyde-src-root = <path>` - The root path to the header file(s) being analyzed. Affects `defined-in-file` output values by taking out the root path.
- `-hyde-yaml-dir = <path>` - Root directory for YAML validation / update. Required for either hyde-validate or hyde-update modes.

This tool parses the passed header using Clang. To pass arguments to the compiler (e.g., include directories), append them after the `--` token on the command line. For example:

    hyde input_file.hpp -hyde-json -- -x c++ -I/path/to/includes

Alternatively, if you have a compilation database and would like to pass that instead of command-line compiler arguments, you can pass that with `-p`.

While compiling the source file, the non-function macro `ADOBE_TOOL_HYDE` is defined to the value `1`. This can be useful to explicitly omit code from the documentation.

# Examples:

To output JSON:
```./hyde ../test_files/apollo.hpp --```

To validate pre-existing YAML:
```./hyde ../test_files/apollo.json -hyde-yaml-dir=/path/to/output -hyde-validate```

To output updated YAML:
```./hyde ../test_files/apollo.json -hyde-yaml-dir=/path/to/output -hyde-update```
