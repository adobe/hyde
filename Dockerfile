
FROM --platform=linux/x86_64 ubuntu:latest
RUN apt-get -y update && apt-get install -y

RUN apt-get -y install curl gnupg2 software-properties-common ninja-build  apt-utils make

RUN apt-get -y install wget

RUN apt-get -y install git

# Are the build-essential packages needed? This elminates a CMake error
# about /usr/bin/c++ not being found but seems like overkill.
RUN apt-get -y install build-essential

# Install llvm/clang

# This is nesessary because of an issue with the hyde resource-dir. The
# version of clang installed must exactly match the version of clang used
# to build hyde. This is a temporary fix until hyde installs the necessary
# resource directory and encodes the path in the binary.

# If you get an error message about stddef.h or size_t not being found,
# the issue is here. Check where hyde is looking for it's resoruce
# directory with
# `hyde ./test.hpp -- -x c++ -print-resource-dir`

ENV LLVM_VERSION=15

RUN apt-get -y install clang-${LLVM_VERSION}

# The above doesn't setup libc++ header paths correctly. Currently LLVM-15
# doesn't install with llvm.sh in docker. So we install LLVM-16 just for
# the libc++ config!

RUN wget https://apt.llvm.org/llvm.sh
RUN chmod +x llvm.sh
RUN ./llvm.sh 16 all

# set clang ${LLVM_VERSION} to be the version of clang we use when clang/clang++ is invoked
RUN update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-${LLVM_VERSION} 100
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-${LLVM_VERSION} 100

ADD https://cmake.org/files/v3.24/cmake-3.24.0-linux-x86_64.sh /cmake-3.24.0-linux-x86_64.sh
RUN mkdir /opt/cmake
RUN sh /cmake-3.24.0-linux-x86_64.sh --prefix=/opt/cmake --skip-license
RUN ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake

#install hyde dependencies
RUN apt-get -y install libyaml-cpp-dev libboost-system-dev libboost-filesystem-dev

COPY . /usr/src/hyde

# build hyde and run the generate_test_files
WORKDIR /usr/src/hyde
RUN mkdir -p build \
    && cd build \
    && rm -rf *  \
    && cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release \
    && ninja

# install hyde
RUN cp ./build/hyde /usr/bin

# RUN apt-get -y install clang-15

CMD ["./generate_test_files.sh"]
