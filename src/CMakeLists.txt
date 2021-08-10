cmake_minimum_required(VERSION 3.14.0)

include_directories("${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/thrid_party/include")

#build fcontext

if (MSVC AND NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
    enable_language(CXX ASM_MASM)
else()
    enable_language(CXX ASM)
endif()

add_definitions(-DBOOST_CONTEXT_EXPORT)

# OS
if (APPLE)
    set(CPU_ARCH "combined")
    set(ASM_EXT "all_macho_gas.S")
elseif (ANDROID)
    # Android
    if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
        set(CPU_ARCH "arm")
        set(ASM_EXT "aapcs_elf_gas.S")
    elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
        set(CPU_ARCH "arm64")
        set(ASM_EXT "aapcs_elf_gas.S")
    elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "i686")
        set(CPU_ARCH "i386")
        set(ASM_EXT "sysv_elf_gas.S")
    elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
        set(CPU_ARCH "x86_64")
        set(ASM_EXT "sysv_elf_gas.S")
    endif()
elseif (UNIX)
    # PC (x86/x64)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CPU_ARCH "x86_64")
    else()
        set(CPU_ARCH "i386")
    endif()
    set(ASM_EXT "sysv_elf_gas.S")  # Linux/Unix
elseif (WIN32)
    # Windows PC
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CPU_ARCH "x86_64")
    else()
        set(CPU_ARCH "i386")
    endif()

    if (CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(ASM_EXT "ms_pe_clang_gas.S")
    elseif (MSVC)
        set(ASM_EXT "ms_pe_masm.asm")
    elseif (CMAKE_C_COMPILER_ID MATCHES "GNU")
        set(ASM_EXT "ms_pe_gas.S")
    endif()
    
endif()

set(fcontext_src "${PROJECT_SOURCE_DIR}/thrid_party/boost.context/asm/make_${CPU_ARCH}_${ASM_EXT}"
                "${PROJECT_SOURCE_DIR}/thrid_party/boost.context/asm/jump_${CPU_ARCH}_${ASM_EXT}"
                "${PROJECT_SOURCE_DIR}/thrid_party/boost.context/asm/ontop_${CPU_ARCH}_${ASM_EXT}")

#build llhttp
aux_source_directory("${PROJECT_SOURCE_DIR}/thrid_party/llhttp/src" llhttp_src)

#build src
aux_source_directory("${PROJECT_SOURCE_DIR}/src" src)
add_library(llhttp ${llhttp_src})
add_library(sharpen ${src} ${fcontext_src})
add_library(sharpen::sharpen ALIAS sharpen)

if(MSVC)
  target_compile_options(sharpen PRIVATE /W4 /WX)
else()
  target_compile_options(sharpen PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

if(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(sharpen PUBLIC Threads::Threads)
endif()

if(WIN32)
    target_link_libraries(sharpen PRIVATE Ws2_32 Mswsock)
endif()

target_link_libraries(sharpen PRIVATE llhttp)