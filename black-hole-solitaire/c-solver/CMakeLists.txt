CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

INCLUDE ("${CMAKE_SOURCE_DIR}/cmake/shlomif_common_bootstrap.cmake")
SHLOMIF_COMMON_SETUP("${private_mod_path}")

CMAKE_POLICY(SET CMP0054 NEW)

INCLUDE(FindPkgConfig)

# Introduces VERSION , CPACK_PACKAGE_VERSION_MAJOR,
# CPACK_PACKAGE_VERSION_MAJOR, and CPACK_PACKAGE_VERSION_PATCH
READ_VERSION_FROM_VER_TXT()

# This is the equivalent to perform a "make dist"/"make distdir" etc.
SET(CPACK_PACKAGE_NAME "black-hole-solver")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Black Hole Solver")
SET(CPACK_PACKAGE_VENDOR "Shlomi Fish")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

SET (BHS_STATE_STORAGE "BHS_STATE_STORAGE_INTERNAL_HASH" CACHE STRING "The State Storage Type")
SET (FCS_IA_PACK_SIZE 64 CACHE STRING "Size of a single pack in kilo-bytes.")
SET (IA_STATE_PACKS_GROW_BY 32 CACHE STRING "Amount to Grow State Packs By")
SET (base_with_ver "black_hole_solver-[0-9]+\\\\.[0-9]+\\\\.[0-9]+")
SET(CPACK_SOURCE_IGNORE_FILES
    "/(build|B/)"
    "/_CPack_Packages/"
    "/CMakeFiles/"
    "/.deps/"
    "${base_with_ver}(-Source|-Linux)/"
    "${base_with_ver}.tar\\\\.(gz|bz2|Z|lzma|xz)$"
    "\\\\.o$"
    "~$"
    "/board_gen/(pi-make-microsoft-freecell-board|make-microsoft-freecell-board|make-gnome-freecell-board|make-aisleriot-freecell-board)$"
    "/(fc-solve|freecell-solver-range-parallel-solve|freecell-solver-fc-pro-range-solve|freecell-solver-multi-thread-solve|freecell-solver-fork-solve|measure-depth-dep-tests-order-perf)$"
    "/lib(fcs|freecell-solver)\\\\.(a|la)$"
    "\\\\.so(\\\\.[0-9]+)*$"
    "\\\\.(pdf|fo)$"
    "/t/t/.*\\\\.exe$"
    "/t/Presets"
    "/CMakeCache\\\\.txt$"
    "/Makefile$"
    "/CTestTestfile\\\\.cmake$"
    "/cmake_install\\\\.cmake$"
    "/CPackConfig\\\\.cmake$"
    "/CPackSourceConfig\\\\.cmake$"
    "/tags$"
    "/freecell-solver-config$"
    "/libfreecell-solver\\\\.pc$"
    "/([0-9]+)\\\\.board$"
    "/config\\\\.h$"
    "/install_manifest\\\\.txt$"
    "/t/card-test-(parse|render)\\\\.c$"
    "/Testing/"
    "\\\\.swp$"
    "ids-whitelist\\\\.txt"
)


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
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

SET (LIB_BASE "black_hole_solver")
option (FCS_WITH_TEST_SUITE "Also build and run the test suite." ON)
SET (BLACK_HOLE_SOLVER_MODULES "alloc.c" "black_hole_solver.c" "rank_reach_prune")

IF (${BHS_STATE_STORAGE} STREQUAL "BHS_STATE_STORAGE_INTERNAL_HASH")
    LIST(APPEND BLACK_HOLE_SOLVER_MODULES fcs_hash.c)
ENDIF ()

SET (LIBTOKYO_CAB_LIBS)

IF (${BHS_STATE_STORAGE} STREQUAL "BHS_STATE_STORAGE_TOKYO_CAB_HASH")

    LIST(APPEND BLACK_HOLE_SOLVER_MODULES tokyo_cab_hash.c)

    PKG_CHECK_MODULES(TOKYO_CAB REQUIRED "tokyocabinet")

    IF (NOT TOKYO_CAB_FOUND)
        MESSAGE (FATAL "Tokyo cabinet could not be found.")
    ENDIF ()

    SET (LIBTOKYO_CAB_LIBS ${TOKYO_CAB_LIBRARIES})
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
    ADD_DEFINITIONS("-Weverything -Wno-language-extension-token -Wno-format-nonliteral -Wno-padded -Wno-gnu-statement-expression -Wno-unused-parameter -Wno-vla -Wno-extra-semi -Wno-cast-align -Wno-used-but-marked-unused -Wno-cast-qual -Wno-conversion -Wno-sign-compare -Wno-overlength-strings -Wno-comma -Wno-disabled-macro-expansion")
ENDIF ()

IF ("$ENV{FCS_GCC}")
    ADD_DEFINITIONS("-W -Wabi -Waddress -Waggressive-loop-optimizations -Wall -Wattributes -Wbad-function-cast -Wbool-compare -Wbool-operation -Wbuiltin-declaration-mismatch -Wbuiltin-macro-redefined -Wcast-align -Wchar-subscripts -Wchkp -Wclobbered -Wcomment -Wcomments -Wcoverage-mismatch -Wcpp -Wdangling-else -Wdate-time -Wdeprecated -Wdeprecated-declarations -Wdesignated-init -Wdisabled-optimization -Wdiscarded-array-qualifiers -Wdiscarded-qualifiers -Wdiv-by-zero -Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Wduplicate-decl-specifier -Wempty-body -Wendif-labels -Wenum-compare -Wexpansion-to-defined -Wextra -Wformat-contains-nul -Wformat-extra-args -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat-y2k -Wformat-zero-length -Wframe-address -Wfree-nonheap-object -Whsa -Wignored-attributes -Wignored-qualifiers -Wimplicit -Wimplicit-function-declaration -Wimplicit-int -Wincompatible-pointer-types -Winit-self -Winline -Wint-conversion -Wint-in-bool-context -Wint-to-pointer-cast -Winvalid-memory-model -Winvalid-pch -Wjump-misses-init -Wlogical-not-parentheses -Wlogical-op -Wmain -Wmaybe-uninitialized -Wmemset-elt-size -Wmemset-transposed-args -Wmisleading-indentation -Wmissing-braces -Wmissing-declarations -Wmissing-field-initializers -Wmissing-include-dirs -Wmissing-parameter-type -Wmissing-prototypes -Wmultichar -Wnarrowing -Wnested-externs -Wnonnull -Wnonnull-compare -Wnull-dereference -Wodr -Wold-style-declaration -Wold-style-definition -Wopenmp-simd -Woverflow -Woverlength-strings -Woverride-init -Wpacked -Wpacked-bitfield-compat -Wparentheses -Wpointer-arith -Wpointer-compare -Wpointer-sign -Wpointer-to-int-cast -Wpragmas -Wpsabi -Wrestrict -Wreturn-local-addr -Wreturn-type -Wscalar-storage-order -Wsequence-point -Wshadow -Wshift-count-negative -Wshift-count-overflow -Wshift-negative-value -Wsizeof-array-argument -Wsizeof-pointer-memaccess -Wstack-protector -Wstrict-aliasing -Wstrict-prototypes -Wsuggest-attribute=const -Wsuggest-attribute=format -Wsuggest-attribute=noreturn -Wsuggest-attribute=pure -Wsuggest-final-methods -Wsuggest-final-types -Wswitch -Wswitch-bool -Wswitch-default -Wswitch-unreachable -Wsync-nand -Wtautological-compare -Wtrampolines -Wtrigraphs -Wtype-limits -Wuninitialized -Wunknown-pragmas -Wunsafe-loop-optimizations -Wunused -Wunused-but-set-parameter -Wunused-but-set-variable -Wunused-function -Wunused-label -Wunused-local-typedefs -Wunused-macros -Wunused-parameter -Wunused-result -Wunused-value -Wunused-variable -Wvarargs -Wvariadic-macros -Wvector-operation-performance -Wvla -Wvolatile-register-var -Wwrite-strings -Walloc-size-larger-than=9223372036854775807 -Warray-bounds=2 -Wformat-overflow=2 -Wformat-truncation=2 -Wnormalized=nfc -Wshift-overflow=2 -Wstringop-overflow=2 -Wunused-const-variable=2 -Wstrict-overflow=1  -Wno-switch-default -Wno-vla -Wno-inline  -Wno-jump-misses-init -Wno-unused-result -Wno-unsafe-loop-optimizations")
    # -Wimplicit-fallthrough=5
    # -Wno-vla-larger-than

    IF (NOT IS_DEBUG)
        ADD_DEFINITIONS("-D_FORTIFY_SOURCE=2")
    ENDIF ()
ENDIF ()

# So it can find config.h
INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_BINARY_DIR}")
INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_SOURCE_DIR}/include")

ADD_LIBRARY (
    "${LIB_BASE}"
    SHARED
    ${BLACK_HOLE_SOLVER_MODULES}
    )

SET_TARGET_PROPERTIES(
    "${LIB_BASE}" PROPERTIES VERSION 0.1.0 SOVERSION 0
)

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

SET (LIBS "${S_LIB}" "${LIB_BASE}")

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
    LIBRARY DESTINATION "lib${LIB_SUFFIX}"
    ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
)

TARGET_LINK_LIBRARIES("${LIB_BASE}" ${LIBTOKYO_CAB_LIBS})

LIST (APPEND BLACK_HOLE_SOLVER_LIBS "${LIB_BASE}")

IF (BUILD_STATIC_LIBRARY)
    SET (STATIC_LIB "${LIB_BASE}-static")
    ADD_LIBRARY (${STATIC_LIB}
        STATIC
        ${FREECELL_SOLVER_LIB_MODULES}
    )

    LIST (APPEND BLACK_HOLE_SOLVER_LIBS "${STATIC_LIB}")

    SET_TARGET_PROPERTIES(
        "${STATIC_LIB}"
        PROPERTIES OUTPUT_NAME "${LIB_BASE}"
    )
    SET_TARGET_PROPERTIES(
        ${BLACK_HOLE_SOLVER_LIBS}
        PROPERTIES CLEAN_DIRECT_OUTPUT 1
    )
ENDIF ()

INCLUDE("${private_mod_path}/xxhash_wrapper_bootstrap.cmake")
SET(AUTOGENERATED_CONFIG_H "config.h was auto-generated from config.h.in . Do not modify directly")

INCLUDE (CheckTypeSize)
CHECK_TYPE_SIZE("int" INT_SIZE_IN_BYTES)
CHECK_TYPE_SIZE("void*" SIZEOF_VOID_P BUILTIN_TYPES_ONLY)

SET (BHS_EXE "black-hole-solve-resume-api")

ADD_EXECUTABLE(
    "${BHS_EXE}"
    "black_hole_solver_resume_api_main.c"
)

TARGET_LINK_LIBRARIES(
    "${BHS_EXE}"
    "${LIB_BASE}"
)

SET (BHS_EXE "black-hole-solve")

ADD_EXECUTABLE(
    "${BHS_EXE}"
    "black_hole_solver_main.c"
)

TARGET_LINK_LIBRARIES(
    "${BHS_EXE}"
    "${LIB_BASE}"
)

INSTALL (TARGETS "${BHS_EXE}" DESTINATION "bin")

INSTALL(
    FILES
        "include/black-hole-solver/black_hole_solver.h"
        "include/black-hole-solver/bool.h"
        "include/black-hole-solver/fcs_dllexport.h"
    DESTINATION
        "include/black-hole-solver"
)

INSTALL(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/lib${CPACK_PACKAGE_NAME}.pc"
    DESTINATION
        "lib${LIB_SUFFIX}/pkgconfig"
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

ADD_TEST(
    NAME perl_run_tests
    COMMAND "perl" "${CMAKE_CURRENT_SOURCE_DIR}/run-tests.pl"
)

ADD_CUSTOM_TARGET(
    "check"
    "perl" "${CMAKE_CURRENT_SOURCE_DIR}/run-tests.pl"
)


ADD_SUBDIRECTORY ("man")
