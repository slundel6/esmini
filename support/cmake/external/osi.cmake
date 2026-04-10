include_guard()

# ############################### Setting osi libraries ##############################################################

macro(set_osi_libs)

    set(OSI_CONFIG_RESOLVED FALSE)

    # New Linux CI artifact layout:
    #   externals/osi/linux/osi-deps
    #   externals/osi/linux/build-osi-deps/vcpkg_installed/x64-linux
    if(LINUX)
        set(OSI_DEPS_PREFIX
            ${EXTERNALS_OSI_OS_SPECIFIC_PATH}/osi-deps)
        set(OSI_VCPKG_PREFIX
            ${EXTERNALS_OSI_OS_SPECIFIC_PATH}/build-osi-deps/vcpkg_installed/x64-linux)
        set(OSI_CONFIG_FILE
            ${OSI_DEPS_PREFIX}/lib/cmake/open_simulation_interface-3/open_simulation_interface-config.cmake)

        if(EXISTS
           ${OSI_CONFIG_FILE}
           AND EXISTS
               ${OSI_VCPKG_PREFIX}/share/protobuf/protobuf-config.cmake)
            file(GLOB OSI_RELEASE_TRANSITIVE_LIBS
                "${OSI_VCPKG_PREFIX}/lib/libabsl_*.a"
                "${OSI_VCPKG_PREFIX}/lib/libutf8*.a"
                "${OSI_VCPKG_PREFIX}/lib/liblz4*.a"
                "${OSI_VCPKG_PREFIX}/lib/libzstd*.a")
            file(GLOB OSI_DEBUG_TRANSITIVE_LIBS
                "${OSI_VCPKG_PREFIX}/debug/lib/libabsl_*.a"
                "${OSI_VCPKG_PREFIX}/debug/lib/libutf8*.a"
                "${OSI_VCPKG_PREFIX}/debug/lib/liblz4*.a"
                "${OSI_VCPKG_PREFIX}/debug/lib/libzstd*.a")

            if(CMAKE_CONFIGURATION_TYPES)
                set(OSI_LIBRARIES
                    debug     ${OSI_DEPS_PREFIX}/lib/libopen_simulation_interface_pic.a
                    debug     -Wl,--start-group
                    debug     ${OSI_VCPKG_PREFIX}/debug/lib/libprotobufd.a
                    debug     ${OSI_DEBUG_TRANSITIVE_LIBS}
                    debug     -Wl,--end-group
                    optimized ${OSI_DEPS_PREFIX}/lib/libopen_simulation_interface_pic.a
                    optimized -Wl,--start-group
                    optimized ${OSI_VCPKG_PREFIX}/lib/libprotobuf.a
                    optimized ${OSI_RELEASE_TRANSITIVE_LIBS}
                    optimized -Wl,--end-group)
            elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(OSI_LIBRARIES
                    ${OSI_DEPS_PREFIX}/lib/libopen_simulation_interface_pic.a
                    -Wl,--start-group
                    ${OSI_VCPKG_PREFIX}/debug/lib/libprotobufd.a
                    ${OSI_DEBUG_TRANSITIVE_LIBS}
                    -Wl,--end-group)
            else()
                set(OSI_LIBRARIES
                    ${OSI_DEPS_PREFIX}/lib/libopen_simulation_interface_pic.a
                    -Wl,--start-group
                    ${OSI_VCPKG_PREFIX}/lib/libprotobuf.a
                    ${OSI_RELEASE_TRANSITIVE_LIBS}
                    -Wl,--end-group)
            endif()

            if(EXISTS
               ${OSI_DEPS_PREFIX}/VERSION)
                file(READ ${OSI_DEPS_PREFIX}/VERSION OSI_VERSION)
                string(STRIP "${OSI_VERSION}" OSI_VERSION)
            endif()

            message(STATUS "OSI target: split CI artifact static libraries (version: ${OSI_VERSION})")
            set(OSI_CONFIG_RESOLVED TRUE)
        endif()
    endif()

    if(NOT OSI_CONFIG_RESOLVED)
        if(APPLE)
            if(DYN_PROTOBUF)
                set(OSI_LIBRARIES
                    ${EXTERNALS_OSI_LIBRARY_PATH}/release/libopen_simulation_interface.dylib
                    ${EXTERNALS_OSI_LIBRARY_PATH}/release/libprotobuf.dylib)
            else()
                set(OSI_LIBRARIES
                    ${EXTERNALS_OSI_LIBRARY_PATH}/release/libopen_simulation_interface_pic.a
                    ${EXTERNALS_OSI_LIBRARY_PATH}/release/libprotobuf.a)
            endif()

        elseif(LINUX)
            if(DYN_PROTOBUF)
                set(OSI_LIBRARIES
                    optimized ${EXTERNALS_OSI_LIBRARY_PATH}/release/libopen_simulation_interface.so
                    optimized ${EXTERNALS_OSI_LIBRARY_PATH}/release/libprotobuf.so
                    debug     ${EXTERNALS_OSI_LIBRARY_PATH}/debug/libopen_simulation_interface.so
                    debug     ${EXTERNALS_OSI_LIBRARY_PATH}/debug/libprotobufd.so)
            else()
                if(EXISTS
                   ${EXTERNALS_OSI_LIBRARY_PATH}/release/libopen_simulation_interface_pic.a)
                    set(OSI_LIBRARIES
                        optimized ${EXTERNALS_OSI_LIBRARY_PATH}/release/libopen_simulation_interface_pic.a
                        optimized ${EXTERNALS_OSI_LIBRARY_PATH}/release/libprotobuf.a
                        debug     ${EXTERNALS_OSI_LIBRARY_PATH}/debug/libopen_simulation_interface_pic.a
                        debug     ${EXTERNALS_OSI_LIBRARY_PATH}/debug/libprotobufd.a)
                else()
                    # Fallback for split CI layout if package configs are absent.
                    set(OSI_DEPS_PREFIX
                        ${EXTERNALS_OSI_OS_SPECIFIC_PATH}/osi-deps)
                    set(OSI_VCPKG_PREFIX
                        ${EXTERNALS_OSI_OS_SPECIFIC_PATH}/build-osi-deps/vcpkg_installed/x64-linux)

                    file(GLOB OSI_RELEASE_TRANSITIVE_LIBS
                        "${OSI_VCPKG_PREFIX}/lib/libabsl_*.a"
                        "${OSI_VCPKG_PREFIX}/lib/libutf8*.a"
                        "${OSI_VCPKG_PREFIX}/lib/liblz4*.a"
                        "${OSI_VCPKG_PREFIX}/lib/libzstd*.a")
                    file(GLOB OSI_DEBUG_TRANSITIVE_LIBS
                        "${OSI_VCPKG_PREFIX}/debug/lib/libabsl_*.a"
                        "${OSI_VCPKG_PREFIX}/debug/lib/libutf8*.a"
                        "${OSI_VCPKG_PREFIX}/debug/lib/liblz4*.a"
                        "${OSI_VCPKG_PREFIX}/debug/lib/libzstd*.a")

                    if(CMAKE_CONFIGURATION_TYPES)
                        set(OSI_LIBRARIES
                            debug     ${OSI_DEPS_PREFIX}/lib/libopen_simulation_interface_pic.a
                            debug     -Wl,--start-group
                            debug     ${OSI_VCPKG_PREFIX}/debug/lib/libprotobufd.a
                            debug     ${OSI_DEBUG_TRANSITIVE_LIBS}
                            debug     -Wl,--end-group
                            optimized ${OSI_DEPS_PREFIX}/lib/libopen_simulation_interface_pic.a
                            optimized -Wl,--start-group
                            optimized ${OSI_VCPKG_PREFIX}/lib/libprotobuf.a
                            optimized ${OSI_RELEASE_TRANSITIVE_LIBS}
                            optimized -Wl,--end-group)
                    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
                        set(OSI_LIBRARIES
                            ${OSI_DEPS_PREFIX}/lib/libopen_simulation_interface_pic.a
                            -Wl,--start-group
                            ${OSI_VCPKG_PREFIX}/debug/lib/libprotobufd.a
                            ${OSI_DEBUG_TRANSITIVE_LIBS}
                            -Wl,--end-group)
                    else()
                        set(OSI_LIBRARIES
                            ${OSI_DEPS_PREFIX}/lib/libopen_simulation_interface_pic.a
                            -Wl,--start-group
                            ${OSI_VCPKG_PREFIX}/lib/libprotobuf.a
                            ${OSI_RELEASE_TRANSITIVE_LIBS}
                            -Wl,--end-group)
                    endif()
                endif()
            endif()

        elseif(MSVC)
            set(OSI_LIBRARIES
                optimized ${EXTERNALS_OSI_LIBRARY_PATH}/release/open_simulation_interface_pic.lib
                optimized ${EXTERNALS_OSI_LIBRARY_PATH}/release/libprotobuf.lib
                debug     ${EXTERNALS_OSI_LIBRARY_PATH}/debug/open_simulation_interface_pic.lib
                debug     ${EXTERNALS_OSI_LIBRARY_PATH}/debug/libprotobufd.lib)
        endif()
    endif()
endmacro()
