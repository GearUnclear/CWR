if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()
if(NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "OUTPUT_FILE is required")
endif()
if(NOT DEFINED BUILD_TYPE)
    set(BUILD_TYPE "")
endif()

execute_process(
    COMMAND git -c safe.directory=${SOURCE_DIR} rev-parse --short=7 HEAD
    WORKING_DIRECTORY ${SOURCE_DIR}
    OUTPUT_VARIABLE GIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT GIT_SHA)
    set(GIT_SHA "unknown")
endif()

set(VERSION_TAG "$ENV{OFPR_BUILD_VERSION_TAG}")
if(VERSION_TAG AND NOT VERSION_TAG MATCHES "^[A-Za-z0-9._-]+$")
    message(FATAL_ERROR "OFPR_BUILD_VERSION_TAG must be a simple token (letters/digits/.-_), got '${VERSION_TAG}'")
endif()

if(VERSION_TAG STREQUAL "")
    set(EFFECTIVE_TAG "${GIT_SHA}")
    set(TAG_SOURCE "git")
    set(RELEASE_BUILD "false")
elseif(VERSION_TAG MATCHES "^[Rr][Ee][Ll][Ee][Aa][Ss][Ee]$")
    set(EFFECTIVE_TAG "release")
    set(TAG_SOURCE "release")
    set(RELEASE_BUILD "true")
else()
    set(EFFECTIVE_TAG "${VERSION_TAG}")
    set(TAG_SOURCE "env")
    set(RELEASE_BUILD "false")
endif()

set(DISPLAY_TAG "${EFFECTIVE_TAG}")
if(DISPLAY_TAG STREQUAL "")
    set(DISPLAY_TAG "<none>")
endif()
message(STATUS "OFPR build: git=${GIT_SHA} version-tag=${DISPLAY_TAG} source=${TAG_SOURCE}")

set(CONTENT [=[
#pragma once

namespace Poseidon::BuildInfo
{
inline constexpr char GitSha[] = "@GIT_SHA@";
inline constexpr char VersionTag[] = "@EFFECTIVE_TAG@";
inline constexpr char BuildType[] = "@BUILD_TYPE@";
inline constexpr bool ReleaseBuild = @RELEASE_BUILD@;
} // namespace Poseidon::BuildInfo
]=])

string(CONFIGURE "${CONTENT}" CONTENT @ONLY)

if(EXISTS "${OUTPUT_FILE}")
    file(READ "${OUTPUT_FILE}" OLD_CONTENT)
else()
    set(OLD_CONTENT "")
endif()

if(NOT CONTENT STREQUAL OLD_CONTENT)
    get_filename_component(OUTPUT_DIR "${OUTPUT_FILE}" DIRECTORY)
    file(MAKE_DIRECTORY "${OUTPUT_DIR}")
    file(WRITE "${OUTPUT_FILE}" "${CONTENT}")
endif()

# Build provenance (build_id, triBuildId): which source the binary was built
# from, uncommitted work included, so two builds of one commit with different
# local changes never report the same id. It has its own header so that a
# source edit recompiles one small file (Core/BuildProvenance.cpp), not every
# file that reads BuildInfo.hpp. The target runs on every build.
#   build id = git describe --always --dirty --abbrev=8, then
#              "+<first 12 hex of a SHA-256>" when engine/ or apps/ differ
#              from HEAD: the hash covers `git diff HEAD --binary -- engine
#              apps` plus the path and SHA-256 of every untracked, not ignored
#              file there.
if(DEFINED OUTPUT_ID_FILE)
    execute_process(
        COMMAND git -c safe.directory=${SOURCE_DIR} describe --always --dirty --abbrev=8
        WORKING_DIRECTORY ${SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DESCRIBE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(NOT GIT_DESCRIBE)
        set(GIT_DESCRIBE "unknown")
    endif()
    execute_process(
        COMMAND git -c safe.directory=${SOURCE_DIR} -c core.quotepath=off diff HEAD --binary -- engine apps
        WORKING_DIRECTORY ${SOURCE_DIR}
        OUTPUT_VARIABLE TREE_DIFF
        ERROR_QUIET
    )
    execute_process(
        COMMAND git -c safe.directory=${SOURCE_DIR} -c core.quotepath=off ls-files --others --exclude-standard -- engine apps
        WORKING_DIRECTORY ${SOURCE_DIR}
        OUTPUT_VARIABLE UNTRACKED
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    set(TREE_STATE "${TREE_DIFF}")
    if(NOT UNTRACKED STREQUAL "")
        string(REPLACE "\n" ";" UNTRACKED_LIST "${UNTRACKED}")
        foreach(f IN LISTS UNTRACKED_LIST)
            if(EXISTS "${SOURCE_DIR}/${f}" AND NOT IS_DIRECTORY "${SOURCE_DIR}/${f}")
                file(SHA256 "${SOURCE_DIR}/${f}" FILE_HASH)
                string(APPEND TREE_STATE "untracked ${f} ${FILE_HASH}\n")
            endif()
        endforeach()
    endif()
    if(TREE_STATE STREQUAL "")
        set(BUILD_ID "${GIT_DESCRIBE}")
    else()
        string(SHA256 TREE_HASH "${TREE_STATE}")
        string(SUBSTRING "${TREE_HASH}" 0 12 TREE_HASH)
        set(BUILD_ID "${GIT_DESCRIBE}+${TREE_HASH}")
    endif()
    message(STATUS "OFPR build: build_id=${BUILD_ID}")

    set(ID_CONTENT [=[
#pragma once

namespace Poseidon::BuildInfo
{
inline constexpr char BuildId[] = "@BUILD_ID@";
} // namespace Poseidon::BuildInfo
]=])
    string(CONFIGURE "${ID_CONTENT}" ID_CONTENT @ONLY)
    if(EXISTS "${OUTPUT_ID_FILE}")
        file(READ "${OUTPUT_ID_FILE}" OLD_ID_CONTENT)
    else()
        set(OLD_ID_CONTENT "")
    endif()
    if(NOT ID_CONTENT STREQUAL OLD_ID_CONTENT)
        get_filename_component(OUTPUT_ID_DIR "${OUTPUT_ID_FILE}" DIRECTORY)
        file(MAKE_DIRECTORY "${OUTPUT_ID_DIR}")
        file(WRITE "${OUTPUT_ID_FILE}" "${ID_CONTENT}")
    endif()
endif()
