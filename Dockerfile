
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

# install a newer version of cmake than can be found on ubuntu
RUN cd /usr/local/src \ 
    && curl -O https://cmake.org/files/v3.13/cmake-3.13.0.tar.gz \
    && tar xvf cmake-3.13.0.tar.gz \ 
    && cd cmake-3.13.0 \
    && ./bootstrap \
    && make \
    && make install \
    && cd .. \
    && rm -rf cmake*

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