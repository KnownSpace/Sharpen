# Sharpen

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/614c8dd1ad944e42b06bbe0316840c9e)](https://app.codacy.com/gh/KnownSpace/Sharpen?utm_source=github.com&utm_medium=referral&utm_content=KnownSpace/Sharpen&utm_campaign=Badge_Grade_Settings)




```
____  _                                 
/ ___|| |__   __ _ _ __ _ __   ___ _ __  
\___ \| '_ \ / _` | '__| '_ \ / _ \ '_ \ 
 ___) | | | | (_| | |  | |_) |  __/ | | |
|____/|_| |_|\__,_|_|  | .__/ \___|_| |_|
                       |_|

Sharpen is a multithreaded C++ network library
```
#### Requires:
  - C++ Standard >= 11
  - Windows version >= 8 or Linux kernel version >= 2.6.28
  - Cmake version >= 3.14
  - Msvc >= 19.0 or Clang >= 3.5 or GCC >= 4.9

#### To build:
##### Just run `build.sh` or `build.cmd`

#### Use in your projects:
  1. Build Sharpen
  1. Add include directory
  1. Add link target


#### Use in your cmake projects:
```cmake
#Include FetchContent
include(FetchContent)
#Get sharpen
fetchcontent_declare(sharpen GIT_REPOSITORY "sharpen repo url" GIT_TAG "git tag" SOURCE_DIR "libraries directory")
fetchcontent_makeavailable(sharpen)
#Link to your projects
target_link_libraries(your_project sharpen::sharpen)
#Include sharpen header file(optional)
include_directories("${SHARPEN_INCLUDE_DIRS}")
```

#### Licenses:
  - Sharpen(MIT License)
  - Boost.Context(BOOST License)
  - llhttp(MIT License)
