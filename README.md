# jingweiprinter

with qgis for TQ application project for jingwei printer

## Motivation

this service can running on headless standalone mode with qgis server in docker container.

![](doc/images/d3_scene.png)


## Development Environment

- oatpp
- oatpp-curl
- yaml-cpp
- spdlog

### Install dependencies

```bash
mkdir ${your project dir}/cpath -p
cd ${your project dir}/cpath
git clone git@ssh.github.com:oatpp/oatpp.git
git clone git@ssh.github.com:oatpp/oatpp-curl.git

cd oatpp
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target install

cd ../oatpp-curl
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target install

```

## Release Environment


```bash
mkdir ${your project dir}/cpath -p
cd ${your project dir}/cpath
git clone git@ssh.github.com:oatpp/oatpp.git
git clone git@ssh.github.com:oatpp/oatpp-curl.git

cd oatpp
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target install

cd ../oatpp-curl
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target install

```

## windows

### with vcpkg

clone vcpkg to your computer

```bash
git clone https://github.com/microsoft/vcpkg.git

# bootstrap vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat

# put the vcpkg to the path you like

PATH=%PATH%;D:\iProject\cpath\vcpkg

```

cmake options

```
-DCMAKE_TOOLCHAIN_FILE=D:\iProject\cpath\vcpkg\scripts\buildsystems\vcpkg.cmake

```

vcpkg install depends
```
vcpkg install curl:64-windows oatpp:x64-windows oatpp-curl:x64-windows oatpp-zlib:x64-windows
```



make symboliclink

```
New-Item -ItemType symboliclink -Path "D:\iProject\cpath\vcpkg\packages\curl_x64-windows\include\oatpp-1.3.0" -Target "D:\iProject\cpath\vcpkg\packages\oatpp-curl_x64-windows\include\oatpp-1.3.0"
New-Item -ItemType symboliclink -Path "D:\iProject\cpath\vcpkg\packages\curl_x64-windows\lib\oatpp-1.3.0" -Target "D:\iProject\cpath\vcpkg\packages\oatpp-curl_x64-windows\lib\oatpp-1.3.0"
```


### without vcpkg

make a jingweiprinter-deps dir

```shell
mkdir D:\\iProject\\cpath\\jingweiprinter-deps
cd D:\\iProject\\cpath\\jingweiprinter-deps

# clone curl
git clone git@github.com:curl/curl.git

# or download source code tar.gz
wget https://github.com/curl/curl/releases/download/curl-8_13_0/curl-8.13.0.tar.gz

# prepare deps dir
mkdir deps

# clone zlib to dir jingweiprinter-deps
git clone git@github.com:madler/zlib.git
git checkout -b v1.3.1 v1.3.1

## build and install zlib
cd zlib
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX=D:/iProject/cpath/jingweiprinter-deps/deps
cmake --build build --config Release --target INSTALL

# > if install to the `C:/Program Files (x86)/zlib/`， copy all to D:/iProject/cpath/jingweiprinter-deps/deps

# build openssl, but need perl from https://strawberryperl.com/， download a PDL zip
## uncompres to D:/iSoft

# open the Developer Powershell for VS 2022 Preview
## running perl environment
D:\iSoft\strawberry-perl-5.40.0.1-64bit-PDL\portableshell.bat
## check perl
where perl

# clone the openssl
cd D:\\iProject\\cpath\\jingweiprinter-deps
git clone git@github.com:openssl/openssl.git
cd openssl
git checkout -b openssl-3.5.0 openssl-3.5.0
perl Configure VC-WIN64A --prefix=D:/iProject/cpath/jingweiprinter-deps/deps
nmake
nmake test
nmake install

## copy elements of openssl directory to the D:/iProject/cpath/jingweiprinter-deps/deps

# clone and build c-ares
git clone git@github.com:c-ares/c-ares.git
cd .\c-ares\
git checkout -b v1.34.5 v1.34.5
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX=D:/iProject/cpath/jingweiprinter-deps/deps
cmake --build build --config Release --target INSTALL

# clone and build libssh2
git clone git@github.com:libssh2/libssh2.git
cd libssh2
git checkout -b libssh2-1.11.1 libssh2-1.11.1

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX=D:/iProject/cpath/jingweiprinter-deps/deps
cmake --build build --config Release --target INSTALL


# build libcurl
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DBUILD_SHARED_LIBS=ON -DCURL_USE_OPENSSL=ON -DENABLE_UNICODE=ON -DCURL_USE_LIBPSL=OFF -DCMAKE_INSTALL_PREFIX=D:\iProject\cpath\jingweiprinter-deps\deps
cmake --build build --config Release --target INSTALL

# build oatpp
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX=D:\iProject\cpath\jingweiprinter-deps\deps
cmake --build build --config Release --target INSTALL


set PKG_CONFIG_PATH=D:\iProject\cpath\jingweiprinter-deps\deps\lib\pkgconfig;%PKG_CONFIG_PATH%
$env:PKG_CONFIG_PATH="D:\iProject\cpath\jingweiprinter-deps\deps\lib\pkgconfig;$env:PKG_CONFIG_PATH"
pkg-config --modversion libcurl
git clone git@github.com:oatpp/oatpp-curl.git
cd .\oatpp-curl
git checkout -b 1.3.0 1.3.0
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DOPENSSL_ROOT_DIR="D:\iProject\cpath\jingweiprinter-deps\deps" -DOPENSSL_INCLUDE_DIR="D:\iProject\cpath\jingweiprinter-deps\deps\include\openssl" -DOPENSSL_CRYPTO_LIBRARY="D:\iProject\cpath\jingweiprinter-deps\deps\lib\libcrypto.lib" -DOPENSSL_SSL_LIBRARY="D:\iProject\cpath\jingweiprinter-deps\deps\lib\libssl.lib" -DCMAKE_INSTALL_PREFIX="D:\iProject\cpath\jingweiprinter-deps\deps"
cmake --build build --config Release --target INSTALL
```