
FROM ubuntu:latest
RUN apt-get -y update && apt-get install -y

RUN apt-get -y install curl gnupg2 software-properties-common ninja-build  apt-utils make

# install clang/llvm 8 
RUN curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN apt-add-repository "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-8 main"
RUN apt-get -y update && apt-get install -y
RUN apt-get -y install libllvm8 llvm-8 llvm-8-dev
RUN apt-get -y install clang-8 clang-tools-8 libclang-common-8-dev libclang-8-dev libclang1-8
RUN apt-get -y install libc++-8-dev libc++abi-8-dev

#set clang 8 to be the version of clang we use when clang/clang++ is invoked
RUN update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-8 100
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-8 100

ADD https://cmake.org/files/v3.13/cmake-3.13.0-Linux-x86_64.sh /cmake-3.13.0-Linux-x86_64.sh
RUN mkdir /opt/cmake
RUN sh /cmake-3.13.0-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
RUN ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake

#install hyde dependencies 
RUN apt-get -y install libyaml-cpp-dev libboost-system-dev libboost-filesystem-dev

COPY . /usr/src/hyde

# build hyde and run the generate_test_files
WORKDIR /usr/src/hyde
RUN mkdir -p build \
    && cd build \
    && rm -rf *  \
    && cmake .. -GNinja \
    && ninja 
WORKDIR /usr/src/hyde
CMD ["./generate_test_files.sh"]