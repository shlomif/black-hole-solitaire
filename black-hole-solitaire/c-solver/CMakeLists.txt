CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE( "${CMAKE_CURRENT_SOURCE_DIR}/Common.cmake" )

INCLUDE(CPack)
INCLUDE(CheckCCompilerFlag)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# This is the equivalent to perform a "make dist"/"make distdir" etc.
SET(CPACK_PACKAGE_NAME "black_hole_solver")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Black Hole Solver")
SET(CPACK_PACKAGE_VENDOR "Shlomi Fish")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt")

# Process and extract the version number.
FILE( READ "ver.txt" VERSION)

CHOMP (VERSION)

STRING (REGEX MATCHALL "([0-9]+)" VERSION_DIGITS "${VERSION}")

LIST(GET VERSION_DIGITS 0 CPACK_PACKAGE_VERSION_MAJOR)
LIST(GET VERSION_DIGITS 1 CPACK_PACKAGE_VERSION_MINOR)
LIST(GET VERSION_DIGITS 2 CPACK_PACKAGE_VERSION_PATCH)

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

SET (BHS_STATE_STORAGE "BHS_STATE_STORAGE_INTERNAL_HASH" CACHE STRING "The State Storage Type")
SET (FCS_IA_PACK_SIZE 64 CACHE STRING "Size of a single pack in kilo-bytes.")
SET (IA_STATE_PACKS_GROW_BY 32 CACHE STRING "Amount to Grow State Packs By")
SET (base_with_ver "black_hole_solver-[0-9]+\\\\.[0-9]+\\\\.[0-9]+")
SET(CPACK_SOURCE_IGNORE_FILES
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
    "/\\\\.svn/"
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
    "ids-whitelist\\\\.txt"
)

SET (LIB_BASE "black_hole_solver")

SET (BLACK_HOLE_SOLVER_MODULES "alloc.c" "${LIB_BASE}.c")

IF (${BHS_STATE_STORAGE} STREQUAL "BHS_STATE_STORAGE_INTERNAL_HASH")

    LIST(APPEND BLACK_HOLE_SOLVER_MODULES fcs_hash.c)

ENDIF (${BHS_STATE_STORAGE} STREQUAL "BHS_STATE_STORAGE_INTERNAL_HASH")


IF (${BHS_STATE_STORAGE} STREQUAL "BHS_STATE_STORAGE_TOKYO_CAB_HASH")

    LIST(APPEND BLACK_HOLE_SOLVER_MODULES tokyo_cab_hash.c)

ENDIF (${BHS_STATE_STORAGE} STREQUAL "BHS_STATE_STORAGE_TOKYO_CAB_HASH")

SET(COMPILER_FLAGS_TO_CHECK 
    "-Wall" "-Werror=implicit-function-declaration" "-fvisibility=hidden"
)

IF (CPU_ARCH)
    LIST(APPEND COMPILER_FLAGS_TO_CHECK "-march=${CPU_ARCH}")
ENDIF(CPU_ARCH)

IF (OPTIMIZATION_OMIT_FRAME_POINTER)
    LIST(APPEND COMPILER_FLAGS_TO_CHECK "-fomit-frame-pointer")
ENDIF(OPTIMIZATION_OMIT_FRAME_POINTER)

IF (CMAKE_BUILD_TYPE STREQUAL debug)
    LIST(APPEND COMPILER_FLAGS_TO_CHECK "-DFCS_COMPILE_DEBUG_FUNCTIONS=1")
ENDIF (CMAKE_BUILD_TYPE STREQUAL debug)

IF (CMAKE_BUILD_TYPE STREQUAL release)
    LIST(APPEND COMPILER_FLAGS_TO_CHECK "-flto")
ENDIF (CMAKE_BUILD_TYPE STREQUAL release)

SET (IDX 1)
FOREACH (CFLAG_TO_CHECK ${COMPILER_FLAGS_TO_CHECK})
    SET (FLAG_EXISTS_VAR "FLAG_EXISTS_${IDX}")
    MATH (EXPR IDX "${IDX} + 1")
    CHECK_C_COMPILER_FLAG("${CFLAG_TO_CHECK}" ${FLAG_EXISTS_VAR})
    IF (${FLAG_EXISTS_VAR})
        ADD_DEFINITIONS(${CFLAG_TO_CHECK})
        LIST(APPEND MY_LINK_FLAGS "${CFLAG_TO_CHECK}")
    ENDIF (${FLAG_EXISTS_VAR})
ENDFOREACH(CFLAG_TO_CHECK)

SET (MY_EXE_FLAGS)
FOREACH (CFLAG_TO_CHECK "-fwhole-program")
    SET (FLAG_EXISTS_VAR "FLAG_EXISTS_${IDX}")
    MATH (EXPR IDX "${IDX} + 1")
    CHECK_C_COMPILER_FLAG("${CFLAG_TO_CHECK}" ${FLAG_EXISTS_VAR})
    IF (${FLAG_EXISTS_VAR})
        LIST(APPEND MY_EXE_FLAGS "${CFLAG_TO_CHECK}")
    ENDIF (${FLAG_EXISTS_VAR})
ENDFOREACH(CFLAG_TO_CHECK)

# So it can find config.h
INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY (
    "${LIB_BASE}"
    SHARED
    ${BLACK_HOLE_SOLVER_MODULES}
    )

LIST (APPEND BLACK_HOLE_SOLVER_LIBS "${LIB_BASE}")

IF (BUILD_STATIC_LIBRARY)
    SET (STATIC_LIB "${LIB_BASE}-static")
    ADD_LIBRARY (${STATIC_LIB}
        STATIC
        ${FREECELL_SOLVER_LIB_MODULES}
    )

    LIST (APPEND BLACK_HOLE_SOLVER_LIBS "${$STATIC_LIB}")

    SET_TARGET_PROPERTIES(
        "${STATIC_LIB}"
        PROPERTIES OUTPUT_NAME "${LIB_BASE}"
    )
    SET_TARGET_PROPERTIES(
        ${BLACK_HOLE_SOLVER_LIBS}
        PROPERTIES CLEAN_DIRECT_OUTPUT 1
    )
ENDIF (BUILD_STATIC_LIBRARY)

SET(AUTOGENERATED_CONFIG_H "config.h was auto-generated from config.h.in . Do not modify directly")

ADD_EXECUTABLE(
    "black-hole-solve"
    "black_hole_solver_main.c"
)

TARGET_LINK_LIBRARIES(
    "black-hole-solve"
    "${LIB_BASE}"
)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    )

# Rebuild config.h if ver.txt has changed.
ADD_CUSTOM_COMMAND(
    OUTPUT "config.h.in"
    DEPENDS "ver.txt"
    COMMAND "touch"
    ARGS "config.h.in"
)

ADD_CUSTOM_TARGET(
    "test"
    "perl" "${CMAKE_CURRENT_SOURCE_DIR}/run-tests.pl"
)

ADD_DEPENDENCIES(
    "test"
    "all"
)

