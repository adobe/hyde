# What is `hyde`

`hyde` is a utility that facilitates documenting C++. The tool is unique from existing documentation utilities in the following ways:

- **Clang based**: In order to properly document your C++, `hyde` compiles it using Clang's excellent [libTooling](https://clang.llvm.org/docs/LibTooling.html) library. Therefore, as the C++ language evolves, so too will `hyde`.
- **Out-of-line**: Many tools rely on documentation placed inline within source as long-form comments. While these seem appealing at first blush, they suffer from two big drawbacks. First, there is nothing keeping the comments from falling out of sync from the elements they document. Secondly (and ironically), experienced users of these libraries eventually find inline documentation to be more of a distraction than a help, cluttering code with comments they no longer read.
- **Jekyll compatible**: `hyde` does not produce pretty-printed output. Rather, it produces well structured Markdown files that contain YAML front-matter. These files can then be consumed by other tools (like Jekyll) to customize the structure and layout of the final documentation.
- **Schema enforcement**: Because of the highly structured nature of the output, `hyde` is able to compare pre-existing documentation files against the current state of your C++ sources. Library developers can use `hyde`'s _update_ mode to facilitate updating documentation against the state of sources. Build engineers can use `hyde`'s _validate_ mode to make sure changes to a code base are accurately reflected in the latest documentation. In the end, the documentation stays true to the code with minimal effort.
- **Adaptable**: While `hyde`'s primary purpose at this point is to output and enforce documentation, the tool can also be used to output AST-based information about your code as a JSON-based IR. This makes room for additional tools to be build atop what `hyde` is able to produce, or additional emitters can be added natively to the tool.

# Example Output

`hyde` produces intermediate documentation files that the developer then fills in with additional details as necessary. The files are then fed through a static site generation tool (like Jekyll) to produce [output like this](http://stlab.cc/libraries/stlab2Fcopy_on_write.hpp/copy_on_write3CT3E/).

# Requirements

## macOS

- Homebrew
    - `brew install cmake`
    - `brew install ninja` (optional)

## Linux

(Note: only tested on ubuntu bionic so far)

- Apt
    - `sudo apt-get install libyaml-cpp-dev`

# How to Build

- clone this repo
- `cd hyde`
- `git submodule update --init`
- `mkdir build`
- `cd build`
- `cmake .. -GNinja` (or `-GXcode`, etc.)
- `ninja` (or whatever your IDE does)

LLVM/Clang are declared as a dependency in the project's `CMakeLists.txt` file, and will be downloaded and made available to the project automatically.

# Using Docker

```
VOLUME="hyde"
docker build --tag $VOLUME .

docker run --platform linux/x86_64 --mount type=bind,source="$(pwd)",target=/mnt/host \
    --tty --interactive \
    hyde bash
```

# Parameters and Flags

There are several modes under which the tool can run:

- `-hyde-json` - (default) Output an analysis dump of the input file as JSON
- `-hyde-validate` - Validate existing YAML documentation
- `-hyde-update` - Write updated YAML documentation

- `-hyde-src-root = <path>` - The root path to the header file(s) being analyzed. Affects `defined_in_file` output values by taking out the root path.
- `-hyde-yaml-dir = <path>` - Root directory for YAML validation / update. Required for either hyde-validate or hyde-update modes.

- `--use-system-clang` - Autodetect and use necessary resource directories and include paths

- `--fixup-hyde-subfield` - As of Hyde v0.1.5, all hyde fields are under a top-level `hyde` subfield in YAML output. This flag will update older hyde documentation that does not have this subfield by creating it, then moving all top-level fields except `title` and `layout` under it. This flag is intended to be used only once during the migration of older documentation from the non-subfield structure to the subfield structure.

This tool parses the passed header using Clang. To pass arguments to the compiler (e.g., include directories), append them after the `--` token on the command line. For example:

    hyde input_file.hpp -hyde-json -use-system-clang -- -x c++ -I/path/to/includes

Alternatively, if you have a compilation database and would like to pass that instead of command-line compiler arguments, you can pass that with `-p`.

While compiling the source file, the non-function macro `ADOBE_TOOL_HYDE` is defined to the value `1`. This can be useful to explicitly omit code from the documentation.

# Examples:

To output JSON:
```./hyde -use-system-clang ../test_files/classes.cpp --```

To validate pre-existing YAML:
```./hyde -use-system-clang -hyde-yaml-dir=/path/to/output -hyde-validate ../test_files/classes.cpp```

To output updated YAML:
```./hyde -use-system-clang -hyde-yaml-dir=/path/to/output -hyde-update ../test_files/classes.cpp```
