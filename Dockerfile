
FROM --platform=linux/x86_64 ubuntu:latest
RUN apt-get -y update && apt-get install -y

RUN apt-get -y install curl gnupg2 software-properties-common ninja-build  apt-utils make

RUN apt-get -y install wget

# Install llvm/clang 13

RUN wget https://apt.llvm.org/llvm.sh
RUN chmod +x llvm.sh
RUN ./llvm.sh 13 all

# set clang 13 to be the version of clang we use when clang/clang++ is invoked
RUN update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-13 100
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-13 100

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
    && cmake .. -GNinja \
    && ninja 

# install hyde
RUN cp ./build/hyde /usr/bin

CMD ["./generate_test_files.sh"]
