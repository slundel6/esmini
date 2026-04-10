include_guard()

# ############################### Setting osi libraries ##############################################################

macro(set_osi_libs)

    # New Linux CI artifact layout:
    #   externals/osi/<platform>/osi-deps
    #   externals/osi/linux/build-osi-deps/vcpkg_installed/<vcpkg_platform>

    set(OSI_DEPS_PREFIX
        ${EXTERNALS_OSI_OS_SPECIFIC_PATH}/osi-deps)

    # Search in vcpkg for protobuf dependencies (abseil, utf8, lz4 and zstd)
    set(FULL_RELEASE_PATTERNS "")
    set(FULL_DEBUG_PATTERNS "")

    set(LIB_SEARCH_PATTERNS "libabsl_*.a" "libutf8*.a" "liblz4*.a" "libzstd*.a")
    foreach(PATTERN ${LIB_SEARCH_PATTERNS})
        list(APPEND FULL_RELEASE_PATTERNS "${EXTERNALS_OSI_OS_SPECIFIC_VCPKG_PATH}/lib/${PATTERN}")
        list(APPEND FULL_DEBUG_PATTERNS "${EXTERNALS_OSI_OS_SPECIFIC_VCPKG_PATH}/debug/lib/${PATTERN}")
    endforeach()

    file(GLOB OSI_RELEASE_TRANSITIVE_LIBS ${FULL_RELEASE_PATTERNS})
    file(GLOB OSI_DEBUG_TRANSITIVE_LIBS ${FULL_DEBUG_PATTERNS})

    if(CMAKE_CONFIGURATION_TYPES)
        set(OSI_LIBRARIES
            debug     ${OSI_DEPS_PREFIX}/lib/libopen_simulation_interface_pic.a
            debug     -Wl,--start-group
            debug     ${EXTERNALS_OSI_OS_SPECIFIC_VCPKG_PATH}/debug/lib/libprotobufd.a
            debug     ${OSI_DEBUG_TRANSITIVE_LIBS}
            debug     -Wl,--end-group
            optimized ${OSI_DEPS_PREFIX}/lib/libopen_simulation_interface_pic.a
            optimized -Wl,--start-group
            optimized ${EXTERNALS_OSI_OS_SPECIFIC_VCPKG_PATH}/lib/libprotobuf.a
            optimized ${OSI_RELEASE_TRANSITIVE_LIBS}
            optimized -Wl,--end-group)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(OSI_LIBRARIES
            ${OSI_DEPS_PREFIX}/lib/libopen_simulation_interface_pic.a
            -Wl,--start-group
            ${EXTERNALS_OSI_OS_SPECIFIC_VCPKG_PATH}/debug/lib/libprotobufd.a
            ${OSI_DEBUG_TRANSITIVE_LIBS}
            -Wl,--end-group)
    else()
        set(OSI_LIBRARIES
            ${OSI_DEPS_PREFIX}/lib/libopen_simulation_interface_pic.a
            -Wl,--start-group
            ${EXTERNALS_OSI_OS_SPECIFIC_VCPKG_PATH}/lib/libprotobuf.a
            ${OSI_RELEASE_TRANSITIVE_LIBS}
            -Wl,--end-group)
    endif()
endmacro()
