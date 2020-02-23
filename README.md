# Drone_using_Raspberry_Pi_ZeroW
TBD

# Dependant components
## Oatpp and Oatpp - WebSocket  
https://github.com/oatpp/oatpp
https://github.com/oatpp/oatpp-websocket  
**You must test to see if you can build and run some Websock examples from  
https://github.com/oatpp/example-websocket   
...on your Raspberry Pi Zero.** Then proceed to next.

# Configuring Cross Compile Environments
s of 2020/02/23, you can't use official toolchain for Raspberry Pi Zero w/ Buster due to this issue.   
This is in open status:  
https://github.com/raspberrypi/tools/issues/102  
You need tool chain under another git repository:  
https://github.com/Pro/raspi-toolchain  
See:  
https://stackoverflow.com/a/58559140/869402

I created a cross compile environment with:
## Build host (PC):
```bash
$ uname -a
Linux kmori-ubuntu1904 5.3.0-29-generic #31-Ubuntu SMP Fri Jan 17 17:27:26 UTC 2020 x86_64 x86_64 x86_64 GNU/Linux
$ cat /etc/os-release 
NAME="Ubuntu"
VERSION="19.10 (Eoan Ermine)"
ID=ubuntu
ID_LIKE=debian
PRETTY_NAME="Ubuntu 19.10"
VERSION_ID="19.10"
HOME_URL="https://www.ubuntu.com/"
SUPPORT_URL="https://help.ubuntu.com/"
BUG_REPORT_URL="https://bugs.launchpad.net/ubuntu/"
PRIVACY_POLICY_URL="https://www.ubuntu.com/legal/terms-and-policies/privacy-policy"
VERSION_CODENAME=eoan
UBUNTU_CODENAME=eoan
```
## Build target (Raspberry Pi Zero):
```bash
$ uname -a
Linux km-drone-air01 4.19.75+ #1270 Tue Sep 24 18:38:54 BST 2019 armv6l GNU/Linux
$ cat /etc/os-release 
PRETTY_NAME="Raspbian GNU/Linux 10 (buster)"
NAME="Raspbian GNU/Linux"
VERSION_ID="10"
VERSION="10 (buster)"
VERSION_CODENAME=buster
ID=raspbian
ID_LIKE=debian
HOME_URL="http://www.raspbian.org/"
SUPPORT_URL="http://www.raspbian.org/RaspbianForums"
BUG_REPORT_URL="http://www.raspbian.org/RaspbianBugs"

```

After creating the cross compiling environments, create a tool chain file *Toolchain-rpi-zero-kmori.cmake*
```cmake
set(SYSROOT_PATH "$ENV{RASPBIAN_ROOTFS}")
set(TOOLCHAIN_HOST "arm-linux-gnueabihf") 

message(STATUS "Using sysroot path: ${SYSROOT_PATH}")

set(TOOLCHAIN_CC "${TOOLCHAIN_HOST}-gcc")
set(TOOLCHAIN_CXX "${TOOLCHAIN_HOST}-g++")

set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_SYSROOT "${SYSROOT_PATH}")

# Define name of the target system
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "armv6l")

# Define the compiler
set(CMAKE_C_COMPILER ${TOOLCHAIN_CC})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_CXX})

# List of library dirs where LD has to look. Pass them directly through gcc. LD_LIBRARY_PATH is not evaluated by arm-*-ld
set(LIB_DIRS 
	"/opt/cross-pi-gcc/arm-linux-gnueabihf/lib"
	"/opt/cross-pi-gcc/lib"
	"${SYSROOT_PATH}/opt/vc/lib"
	"${SYSROOT_PATH}/lib/${TOOLCHAIN_HOST}"
	"${SYSROOT_PATH}/usr/local/lib"
	"${SYSROOT_PATH}/usr/lib/${TOOLCHAIN_HOST}"
	"${SYSROOT_PATH}/usr/lib"
	"${SYSROOT_PATH}/usr/lib/${TOOLCHAIN_HOST}/blas"
	"${SYSROOT_PATH}/usr/lib/${TOOLCHAIN_HOST}/lapack"
)
# You can additionally check the linker paths if you add the flags ' -Xlinker --verbose'
set(COMMON_FLAGS "-I${SYSROOT_PATH}/usr/include ")
FOREACH(LIB ${LIB_DIRS})
	set(COMMON_FLAGS "${COMMON_FLAGS} -L${LIB} -Wl,-rpath-link,${LIB}")
ENDFOREACH()

set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${SYSROOT_PATH}/usr/lib/${TOOLCHAIN_HOST}")

# Explicitly specified -march because arm-linux-gnueabihf's default is 'armv6+fp' which causes Segmentation Fault on my ZeroW.
set(CMAKE_C_FLAGS "-march=armv6 -mfpu=vfp -mfloat-abi=hard ${COMMON_FLAGS}" CACHE STRING "Flags for Raspberry PI 1 B+ Zero")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "Flags for Raspberry PI 1 B+ Zero")

# CMAKE_FIND_ROOT_PATH is where the target system libs/headers to be found.
set(CMAKE_FIND_ROOT_PATH "${CMAKE_INSTALL_PREFIX};${CMAKE_PREFIX_PATH};${CMAKE_SYSROOT}")

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
# These shold be found in BOTH(for findxxx() in CMake. Somehow PATHS were ignored...)
# Assuming there is no library in the same path on the host.
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
# Packages shold be found only in CMAKE_FIND_ROOT_PATH(for me)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

Now append these lines to your profile(e.g ~/.bashrc)
```bash
export RASPBIAN_ROOTFS=$HOME/raspberry/rootfs
export PATH=/opt/cross-pi-gcc/bin:$PATH
export RASPBERRY_VERSION=1
```

You can cross compile your code using cmake specifying this toolchain file.
```bash 
$ cd <your cnamelist.txt location>
$ mkdir build ; cd build
$ cmake -DCMAKE_TOOLCHAIN_FILE=$HOME/raspberry/Toolchain-rpi-zero-kmori.cmake ..
```