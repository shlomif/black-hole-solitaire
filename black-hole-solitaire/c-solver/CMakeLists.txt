CMAKE_MINIMUM_REQUIRED(VERSION 3.15)
PROJECT(black-hole-solver)

INCLUDE ("${CMAKE_SOURCE_DIR}/cmake/shlomif_common_bootstrap.cmake")
SHLOMIF_COMMON_SETUP("${private_mod_path}")
INCLUDE(GNUInstallDirs)

CMAKE_POLICY(SET CMP0054 NEW)
option (FCS_WITH_TEST_SUITE "Also build and run the test suite." ON)
option (FCS_AVOID_TCMALLOC "Avoid linking against Google's tcmalloc")
option (USE_SYSTEM_XXHASH "Link to the system's xxHash instead of the bundled copy")
option (BUILD_STATIC_LIBRARY "Build and install the static library")
option (LINK_TO_STATIC "Link the executables to the static library")
option (DISABLE_APPLYING_RPATH "Disable applying rpath")
option (ENABLE_DISPLAYING_MAX_NUM_PLAYED_CARDS "Allow recording and displaying the maximal number of played/\"moved\" cards. Enabling it may make the non-related run time somewhat slower.")
option (USE_SIGNED_CHARS "-fsigned-char")
option (USE_UNSIGNED_CHARS "-funsigned-char")

INCLUDE(FindPkgConfig)

# Introduces VERSION , CPACK_PACKAGE_VERSION_MAJOR,
# CPACK_PACKAGE_VERSION_MAJOR, and CPACK_PACKAGE_VERSION_PATCH
READ_VERSION_FROM_VER_TXT()

# This is the equivalent to perform a "make dist"/"make distdir" etc.
SET(CPACK_PACKAGE_NAME "black-hole-solver")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Black Hole Solver")
SET(CPACK_PACKAGE_VENDOR "Shlomi Fish")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

SET (BHS_STATE_STORAGE "BHS_STATE_STORAGE_INTERNAL_HASH" CACHE STRING "The State Storage Type")
SET (FCS_IA_PACK_SIZE 64 CACHE STRING "Size of a single pack in kilo-bytes.")
SET (IA_STATE_PACKS_GROW_BY 32 CACHE STRING "Amount to Grow State Packs By ( UNUSED! Kept for backward compatibility. )")
SET (base_with_ver "black_hole_solver-[0-9]+\\\\.[0-9]+\\\\.[0-9]+")
SET(CPACK_SOURCE_GENERATOR "TXZ")
SET(CPACK_SOURCE_IGNORE_FILES
    "/\\\\.git/"
    "/\\\\.tidyall\\\\.d/"
    "/tags$"
    "\\\\.swp$"
    "ids-whitelist\\\\.txt"
    "~$"
)

IF (NOT DISABLE_APPLYING_RPATH)
    ### This is to set the RPATH correctly, so when installed under a prefix
    ### the executables will find the libraries.
    ###
    ### See:
    ###
    ### http://www.cmake.org/Wiki/CMake_RPATH_handling
    ###
    ### (Taken from that wiki page)

    # use, i.e. don't skip the full RPATH for the build tree
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

    # when building, don't use the install RPATH already
    # (but later on when installing)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

    # the RPATH to be used when installing
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")

    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDIF ()
SET (LIB_BASE "black_hole_solver")
SET (BLACK_HOLE_SOLVER_MODULES "can_move.c" "generated/lib.c" "meta_alloc.c" "rank_reach_prune.c")

IF (${BHS_STATE_STORAGE} STREQUAL "BHS_STATE_STORAGE_INTERNAL_HASH")
    LIST(APPEND BLACK_HOLE_SOLVER_MODULES fcs_hash.c)
ENDIF ()

SET (DEP_LIBS)

IF (${BHS_STATE_STORAGE} STREQUAL "BHS_STATE_STORAGE_TOKYO_CAB_HASH")
    PKG_CHECK_MODULES(TOKYO_CAB REQUIRED "tokyocabinet")

    IF (NOT TOKYO_CAB_FOUND)
        MESSAGE (FATAL "Tokyo cabinet could not be found.")
    ENDIF ()

    SET (DEP_LIBS ${TOKYO_CAB_LIBRARIES})
ENDIF ()

IF (USE_SYSTEM_XXHASH)
    LIST(APPEND DEP_LIBS "xxhash")
ENDIF ()

INCLUDE ("${CMAKE_SOURCE_DIR}/cmake/rinutils_bootstrap.cmake")

RINUTILS_SET_UP_FLAGS()

SET(LIBTCMALLOC_LIB_LIST)

# Avoid linking to tcmalloc if the test suite is enabled due to:
# https://github.com/gperftools/gperftools/issues/758
IF ((NOT (FCS_AVOID_TCMALLOC)) AND (NOT (FCS_WITH_TEST_SUITE)) AND ( NOT ((CMAKE_BUILD_TYPE STREQUAL "debug") OR (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo") ) ))
    # Optionally link against Google's TCMalloc if it's available:
    # http://goog-perftools.sourceforge.net/
    # This gives better performance for the threaded programs.
    FIND_LIBRARY(LIBTCMALLOC_LIB tcmalloc)

    IF(NOT ("${LIBTCMALLOC_LIB}" STREQUAL "LIBTCMALLOC_LIB-NOTFOUND"))
        SET(LIBTCMALLOC_LIB_LIST ${LIBTCMALLOC_LIB})
    ENDIF()
ENDIF ()

include(TestBigEndian)
TEST_BIG_ENDIAN(BIG_ENDIAN)
IF (BIG_ENDIAN)
    ADD_DEFINITIONS("-DXXH_CPU_LITTLE_ENDIAN=0")
ELSE ()
    ADD_DEFINITIONS("-DXXH_CPU_LITTLE_ENDIAN=1")
ENDIF ()

# Add the google_hash.cpp if (and only if) it is being used.
#
IF (${BHS_STATE_STORAGE} STREQUAL "BHS_STATE_STORAGE_GOOGLE_SPARSE_HASH")
    LIST(APPEND BLACK_HOLE_SOLVER_MODULES google_hash.cpp)
ENDIF ()

INCLUDE(CPack)

SHLOMIF_ADD_COMMON_C_FLAGS()

SHLOMIF_FINALIZE_FLAGS()

IF ("$ENV{FCS_CLANG}")
    ADD_DEFINITIONS("-Weverything -Wno-language-extension-token -Wno-gnu-statement-expression -Wno-used-but-marked-unused -Wno-padded -Wno-cast-align -Wno-extra-semi-stmt")
    ADD_DEFINITIONS("-Wno-declaration-after-statement")
ENDIF ()

IF ("$ENV{FCS_GCC}")
    ADD_DEFINITIONS("-W -Waddress -Waggressive-loop-optimizations -Wall -Wattributes -Wbad-function-cast -Wbool-compare -Wbool-operation -Wbuiltin-declaration-mismatch -Wbuiltin-macro-redefined -Wcast-align -Wchar-subscripts -Wchkp -Wclobbered -Wcomment -Wcomments -Wcoverage-mismatch -Wcpp -Wdangling-else -Wdate-time -Wdeprecated -Wdeprecated-declarations -Wdesignated-init -Wdisabled-optimization -Wdiscarded-array-qualifiers -Wdiscarded-qualifiers -Wdiv-by-zero -Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Wduplicate-decl-specifier -Wempty-body -Wendif-labels -Wenum-compare -Wexpansion-to-defined -Wextra -Wformat-contains-nul -Wformat-extra-args -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat-y2k -Wformat-zero-length -Wframe-address -Wfree-nonheap-object -Whsa -Wignored-attributes -Wignored-qualifiers -Wimplicit -Wimplicit-function-declaration -Wimplicit-int -Wincompatible-pointer-types -Winit-self -Winline -Wint-conversion -Wint-in-bool-context -Wint-to-pointer-cast -Winvalid-memory-model -Winvalid-pch -Wjump-misses-init -Wlogical-not-parentheses -Wlogical-op -Wmain -Wmaybe-uninitialized -Wmemset-elt-size -Wmemset-transposed-args -Wmisleading-indentation -Wmissing-braces -Wmissing-declarations -Wmissing-field-initializers -Wmissing-include-dirs -Wmissing-parameter-type -Wmissing-prototypes -Wmultichar -Wnarrowing -Wnested-externs -Wnonnull -Wnonnull-compare -Wnull-dereference -Wodr -Wold-style-declaration -Wold-style-definition -Wopenmp-simd -Woverflow -Woverlength-strings -Woverride-init -Wpacked -Wpacked-bitfield-compat -Wparentheses -Wpointer-arith -Wpointer-compare -Wpointer-sign -Wpointer-to-int-cast -Wpragmas -Wpsabi -Wrestrict -Wreturn-local-addr -Wreturn-type -Wscalar-storage-order -Wsequence-point -Wshadow -Wshift-count-negative -Wshift-count-overflow -Wshift-negative-value -Wsizeof-array-argument -Wsizeof-pointer-memaccess -Wstack-protector -Wstrict-aliasing -Wstrict-prototypes -Wsuggest-attribute=const -Wsuggest-attribute=format -Wsuggest-attribute=noreturn -Wsuggest-attribute=pure -Wsuggest-final-methods -Wsuggest-final-types -Wswitch -Wswitch-bool -Wswitch-default -Wswitch-unreachable -Wsync-nand -Wtautological-compare -Wtrampolines -Wtrigraphs -Wtype-limits -Wuninitialized -Wunsafe-loop-optimizations -Wunused -Wunused-but-set-parameter -Wunused-but-set-variable -Wunused-function -Wunused-label -Wunused-local-typedefs -Wunused-macros -Wunused-parameter -Wunused-result -Wunused-value -Wunused-variable -Wvarargs -Wvariadic-macros -Wvector-operation-performance -Wvla -Wvolatile-register-var -Wwrite-strings -Walloc-size-larger-than=9223372036854775807 -Warray-bounds=2 -Wformat-overflow=2 -Wformat-truncation=2 -Wnormalized=nfc -Wshift-overflow=2 -Wstringop-overflow=2 -Wunused-const-variable=2 -Wstrict-overflow=1  -Wno-switch-default -Wno-vla -Wno-inline  -Wno-jump-misses-init -Wno-unused-result -Wno-unsafe-loop-optimizations")
    # -Wimplicit-fallthrough=5
    # -Wno-vla-larger-than

    IF (NOT IS_DEBUG)
        ADD_DEFINITIONS("-D_FORTIFY_SOURCE=2")
    ENDIF ()
ENDIF ()
ADD_DEFINITIONS("-Wno-unknown-pragmas")

IF (USE_UNSIGNED_CHARS)
    ADD_DEFINITIONS("-funsigned-char")
    IF (USE_SIGNED_CHARS)
        MESSAGE(FATAL_ERROR "Cannot have both USE_SIGNED_CHARS and USE_UNSIGNED_CHARS !")
    ENDIF ()
ELSEIF (USE_SIGNED_CHARS)
    ADD_DEFINITIONS("-fsigned-char")
ENDIF ()

# So it can find config.h
INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_BINARY_DIR}")
INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_BINARY_DIR}/generated")
INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_BINARY_DIR}/generated/include")
INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_SOURCE_DIR}/include")

SET (is_parent_gen "${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen-c-lookup-files.pl")
SET (is_parent_args "${is_parent_gen}")
SET (is_parent_output
    "${CMAKE_CURRENT_BINARY_DIR}/can_move.c" "${CMAKE_CURRENT_BINARY_DIR}/can_move.h"
)
ADD_CUSTOM_COMMAND(
    OUTPUT ${is_parent_output}
    COMMAND "${PERL_EXECUTABLE}"
    ARGS ${is_parent_args}
    DEPENDS "${is_parent_gen}"
)
# Execute it right away to avoid missing files/includes
SET(cmd "${PERL_EXECUTABLE}" ${is_parent_args})
EXECUTE_PROCESS(COMMAND ${cmd} RESULT_VARIABLE ret)
IF (NOT ("${ret}" STREQUAL "0"))
    MESSAGE(FATAL_ERROR "${cmd} failed!")
ENDIF ()

IF (ENABLE_DISPLAYING_MAX_NUM_PLAYED_CARDS)
    SET(process_args "--process=none")
ELSE ()
    SET(process_args "--process=no_max_num_played")
ENDIF ()

FIND_PROGRAM(_PYTHON3 "python3")
SET (found_python3 )
IF (NOT "${_PYTHON3}" STREQUAL "_PYTHON3-NOTFOUND")
    SET (found_python3 "1")
    SET (PYTHON3_EXECUTABLE "python3")
ENDIF ()
SET(py3_prog "${CMAKE_CURRENT_SOURCE_DIR}/scripts/source_filter.py")
SET(cmd "${PYTHON3_EXECUTABLE}" "${py3_prog}" ${process_args})
SET(sources)
SET(dests)
FOREACH (src "lib.c" "solver_common.h" "include/black-hole-solver/black_hole_solver.h")
    LIST(APPEND sources "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
    LIST(APPEND dests "${CMAKE_CURRENT_BINARY_DIR}/generated/${src}")
ENDFOREACH()
ADD_CUSTOM_COMMAND(
    OUTPUT ${dests}
    COMMAND ${cmd}
    DEPENDS "${py3_prog}" ${sources}
)
EXECUTE_PROCESS(COMMAND ${cmd} RESULT_VARIABLE ret)
IF (NOT ("${ret}" STREQUAL "0"))
    MESSAGE(FATAL_ERROR "${cmd} failed!")
ENDIF ()
ADD_LIBRARY (
    "${LIB_BASE}"
    SHARED
    ${BLACK_HOLE_SOLVER_MODULES}
)

SET_TARGET_PROPERTIES(
    "${LIB_BASE}" PROPERTIES VERSION 1.0.2 SOVERSION 1
)

SET (LIBS "${LIB_BASE}")
SET (LINKLIB "${LIB_BASE}")

IF (BUILD_STATIC_LIBRARY)
    SET (S_LIB "${LIB_BASE}-static")
    ADD_LIBRARY (
        "${S_LIB}"
        STATIC
        ${BLACK_HOLE_SOLVER_MODULES}
    )
    SET_TARGET_PROPERTIES(
        "${S_LIB}"
        PROPERTIES OUTPUT_NAME "${LIB_BASE}"
    )
    LIST(APPEND LIBS "${S_LIB}")
    IF (LINK_TO_STATIC)
        SET (LINKLIB "${S_LIB}")
    ENDIF ()
ENDIF ()

SET_TARGET_PROPERTIES(
    ${LIBS}
    PROPERTIES CLEAN_DIRECT_OUTPUT 1
)

ADD_LIBRARY (
    "bhs_rank_reach_prune"
    SHARED
    "rank_reach_prune.c"
)

INSTALL (
    TARGETS ${LIBS}
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

TARGET_LINK_LIBRARIES("${LIB_BASE}" ${DEP_LIBS})

IF (WIN32 AND NOT UNIX)
    # This is so on Windows, the .dll's will be installed in the bin/
    # directory as well where the command-line utilities
    # will be able to find them.

    INSTALL(
        TARGETS "${LIB_BASE}"
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )
ENDIF ()

IF (NOT USE_SYSTEM_XXHASH)
    INCLUDE("${private_mod_path}/xxhash_wrapper_bootstrap.cmake")
ENDIF ()

SET(AUTOGENERATED_CONFIG_H "config.h was auto-generated from config.h.in . Do not modify directly")

INCLUDE (CheckTypeSize)
CHECK_TYPE_SIZE("int" INT_SIZE_IN_BYTES)
CHECK_TYPE_SIZE("void*" SIZEOF_VOID_P BUILTIN_TYPES_ONLY)

SET (BHS_EXE "black-hole-solve")

ADD_EXECUTABLE(
    "${BHS_EXE}"
    "single_board_main.c"
)

INSTALL (TARGETS "${BHS_EXE}" DESTINATION "${CMAKE_INSTALL_BINDIR}")

ADD_EXECUTABLE(
    "multi-bhs-solver"
    "multi_solver.c"
)
SET(exes  "${BHS_EXE}" "multi-bhs-solver")
IF (ENABLE_DISPLAYING_MAX_NUM_PLAYED_CARDS)
    ADD_EXECUTABLE(
        "stats-multi-bhs-solver"
        "multi_solver_stats.c"
    )
    LIST(APPEND exes "stats-multi-bhs-solver")
ENDIF ()

FOREACH (exe ${exes})
    TARGET_LINK_LIBRARIES(
        "${exe}"
        "${LINKLIB}"
        ${LIBTCMALLOC_LIB_LIST}
    )
ENDFOREACH()

INSTALL(
    FILES
        "include/black-hole-solver/black_hole_solver.h"
        "include/black-hole-solver/bool.h"
        "include/black-hole-solver/fcs_dllexport.h"
    DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}/black-hole-solver"
)

INSTALL(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/lib${CPACK_PACKAGE_NAME}.pc"
    DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/${CPACK_PACKAGE_NAME}.spec.in
    ${CMAKE_CURRENT_SOURCE_DIR}/${CPACK_PACKAGE_NAME}.spec
)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/lib${CPACK_PACKAGE_NAME}.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/lib${CPACK_PACKAGE_NAME}.pc
    @ONLY
)

# Rebuild config.h if ver.txt has changed.
ADD_CUSTOM_COMMAND(
    OUTPUT "config.h.in"
    DEPENDS "ver.txt"
    COMMAND "touch"
    ARGS "config.h.in"
)

ENABLE_TESTING()

SET (test_tags)
IF (NOT ENABLE_DISPLAYING_MAX_NUM_PLAYED_CARDS)
    LIST(APPEND test_tags ":no_max_num_played")
ENDIF ()
FILE (WRITE "${CMAKE_CURRENT_BINARY_DIR}/TEST_TAGS.txt" "${test_tags}")
ADD_TEST(
    NAME perl_run_tests
    COMMAND "perl" "${CMAKE_CURRENT_SOURCE_DIR}/run-tests.pl"
)
ADD_CUSTOM_TARGET(
    "check"
    "perl" "${CMAKE_CURRENT_SOURCE_DIR}/run-tests.pl"
)

ADD_SUBDIRECTORY ("man")
