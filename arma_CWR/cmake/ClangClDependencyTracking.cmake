# CMake 4.3 defaults clang-cl to GCC-style depfiles using -clang:-MD/-MF.
# ccache 4.13.6 does not restore those depfiles on cache hits, so Ninja records
# zero dependencies and silently keeps stale objects after header changes.
# Use the include trace that ccache preserves and Ninja understands instead.
if(CMAKE_GENERATOR MATCHES "^Ninja")
    foreach(lang C CXX)
        if(CMAKE_${lang}_COMPILER_ID STREQUAL "Clang"
           AND CMAKE_${lang}_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            set(CMAKE_DEPFILE_FLAGS_${lang} "/showIncludes")
            set(CMAKE_${lang}_DEPFILE_FORMAT msvc)
        endif()
    endforeach()
endif()
