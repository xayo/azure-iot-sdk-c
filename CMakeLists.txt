#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)
project(azure_iot_sdks)

# Include the common build rules for the C SDK
include(c-utility/configs/azure_iot_build_rules.cmake)

STRING( REPLACE "-D_POSIX_C_SOURCE=200112L" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS} )

include("configs/azure_iot_sdksFunctions.cmake")

getIoTSDKVersion()
message(STATUS "IoT Client SDK Version = ${IOT_SDK_VERSION}")

set(hsm_type_x509 OFF CACHE BOOL "x509 type of hsm used with the Provisioning client")
set(hsm_type_sastoken OFF CACHE BOOL "tpm type of hsm used with the Provisioning client")
set(hsm_type_custom OFF CACHE BOOL "hsm type of custom used with the Provisioning client")
set(hsm_custom_lib "" CACHE STRING "Full path to custom HSM repo library")

#the following variables are project-wide and can be used with cmake-gui
option(use_amqp "set use_amqp to ON if amqp is to be used, set to OFF to not use amqp" ON)
option(use_http "set use_http to ON if http is to be used, set to OFF to not use http" ON)
option(use_mqtt "set use_mqtt to ON if mqtt is to be used, set to OFF to not use mqtt" ON)
option(run_e2e_tests "set run_e2e_tests to ON to run e2e tests (default is OFF)" OFF)
option(run_unittests "set run_unittests to ON to run unittests (default is OFF)" OFF)
option(run_longhaul_tests "set run_longhaul_tests to ON to run longhaul tests (default is OFF)[if possible, they are always build]" OFF)
option(skip_samples "set skip_samples to ON to skip building samples (default is OFF)[if possible, they are always build]" OFF)
option(build_service_client "controls whether the iothub_service_client is built or not" ON)
option(build_provisioning_service_client "controls whether the provisioning_service_client is built or not" ON)
option(build_python "builds the Python native iothub_client module" OFF)
option(build_javawrapper "builds the native iothub_client library for java C wrapper" OFF)
option(dont_use_uploadtoblob "set dont_use_uploadtoblob to ON if the functionality of upload to blob is to be excluded, OFF otherwise. It requires HTTP" OFF)
option(no_logging "disable logging" OFF)
option(use_installed_dependencies "set use_installed_dependencies to ON to use installed packages instead of building dependencies from submodules" OFF)
option(use_firmware_update "build the Raspberry PI firmware_update sample" OFF)
option(build_as_dynamic "build the IoT SDK libaries as dynamic"  OFF)
option(build_network_e2e "build network E2E tests" OFF)
option(use_prov_client "Enable provisioning client" OFF)
option(use_tpm_simulator "tpm simulator type of hsm used with the provisioning client" OFF)

if(WIN32)
    option(use_openssl "set use_openssl to ON to use OpenSSL." OFF)
else()
    option(use_openssl "set use_openssl to ON to use OpenSSL." ON)
endif()

#openssl samples on Windows need to have a trusted cert set
if(WIN32 AND ${use_openssl})
    option(set_trusted_cert_in_samples "set set_trusted_cert_in_samples to ON to have the samples set the SDK's built-in CA as TrustedCerts" ON)
else()
    option(set_trusted_cert_in_samples "set set_trusted_cert_in_samples to ON to have the samples set the SDK's built-in CA as TrustedCerts" OFF)
endif()

# Set Provisioning Information
if (${use_prov_client})
    if ("${hsm_custom_lib}" STREQUAL "")
        set(hsm_type_x509 ON)
        set(hsm_type_sastoken ON)
    else()
        set(hsm_type_custom ON)
    endif()
endif()

#setting nuget_e2e_tests will only generate e2e tests to run with nuget packages.  Install-packages from Package Manager Console in VS before building the projects
option(nuget_e2e_tests "set nuget_e2e_tests to ON to generate e2e tests to run with nuget packages (default is OFF)" OFF)

#check for conflicting options
if(NOT ${use_http})
    MESSAGE( "Setting dont_use_uploadtoblob to ON because use_http is OFF")
    set(dont_use_uploadtoblob "ON")
    MESSAGE( STATUS "use_http:         " ${use_http} )
    MESSAGE( STATUS "dont_use_uploadtoblob:         " ${dont_use_uploadtoblob} )
endif()

if(${dont_use_uploadtoblob})
    add_definitions(-DDONT_USE_UPLOADTOBLOB)
endif()

if(${no_logging})
    add_definitions(-DNO_LOGGING)
endif()

if(IN_OPENWRT)
    ADD_DEFINITIONS("$ENV{TARGET_LDFLAGS}" "$ENV{TARGET_CPPFLAGS}" "$ENV{TARGET_CFLAGS}")
    INCLUDE_DIRECTORIES("$ENV{TOOLCHAIN_DIR}/usr/include" "$ENV{TARGET_LDFLAGS}" "$ENV{TARGET_CPPFLAGS}" "$ENV{TARGET_CFLAGS}")
endif()

if (LINUX)
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
        # now all static libraries use PIC flag for Python shared lib
        set(CMAKE_C_FLAGS "-fPIC ${CMAKE_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "-fPIC ${CMAKE_CXX_FLAGS}")
    endif()
endif()

if(${run_unittests} OR ${run_e2e_tests})
    include("dependencies-test.cmake")
endif()

#do not add or build any tests of the dependencies
set(original_run_e2e_tests ${run_e2e_tests})
set(original_run_unittests ${run_unittests})

set(run_e2e_tests OFF)
set(run_unittests OFF)

include("dependencies.cmake")

if (${original_run_e2e_tests} OR ${original_run_unittests})
    set(SHARED_UTIL_REAL_TEST_FOLDER ${CMAKE_CURRENT_LIST_DIR}/c-utility/tests/real_test_files CACHE INTERNAL "this is what needs to be included when doing test sources" FORCE)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/c-utility/testtools)
endif()

set_platform_files(${CMAKE_CURRENT_LIST_DIR}/c-utility)

set(run_e2e_tests ${original_run_e2e_tests})
set(run_unittests ${original_run_unittests})

if(WIN32)
    set(LOCK_C_FILE ${SHARED_UTIL_ADAPTER_FOLDER}/lock_win32.c)
    set(THREAD_C_FILE ${SHARED_UTIL_ADAPTER_FOLDER}/threadapi_c11.c)
else()
    set(LOCK_C_FILE ${SHARED_UTIL_ADAPTER_FOLDER}/lock_pthreads.c)
    set(THREAD_C_FILE ${SHARED_UTIL_ADAPTER_FOLDER}/threadapi_pthreads.c)
endif()

#Set CMAKE_INSTALL_LIBDIR if not defined
include(GNUInstallDirs)

if(NOT ${use_amqp} OR NOT ${use_http})
    set (build_service_client OFF)
    message(STATUS "iothub_service_client build is disabled (AMQP and HTTP support are required)")
endif()

if(NOT ${use_http})
    set (build_provisioning_service_client OFF)
    message(STATUS "provisioning_service_client build is disabled (HTTP support is required)")
endif()

if (${use_prov_client})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_PROV_MODULE")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_PROV_MODULE")

    add_subdirectory(provisioning_client)

    if(${build_provisioning_service_client})
        add_subdirectory(provisioning_service_client)
    endif()

endif()

if(${build_service_client})
    add_subdirectory(iothub_service_client)
endif()

if(${run_e2e_tests} OR ${run_longhaul_tests} OR ${nuget_e2e_tests})
    add_subdirectory(testtools)
endif()

add_subdirectory(iothub_client)
add_subdirectory(serializer)

#add the device_auth sample subdirectory
if(${use_prov_client} AND NOT ${skip_samples})
    add_subdirectory(./provisioning_client/samples)

    if(NOT "${build_python}" STREQUAL "OFF")
        add_subdirectory(../provisioning_device_client provisioning_device_client_python)
    endif()
endif()

if(NOT "${build_python}" STREQUAL "OFF")
    add_subdirectory(../device/iothub_client_python python)
    if(${build_service_client})
        add_subdirectory(../service python_service_client)
    endif()
endif()

if("${build_javawrapper}" STREQUAL "ON")
     add_subdirectory(../device/iothub_client_javawrapper javawrapper)
endif()

if(${use_installed_dependencies})
    #Install azure_iot_sdks
    set(package_location "cmake")

    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${IOT_SDK_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_file("configs/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
        COPYONLY
    )

    install(EXPORT azure_iot_sdksTargets
        FILE
            "${PROJECT_NAME}Targets.cmake"
        DESTINATION
            ${package_location}
    )

    install(
        FILES
            "configs/${PROJECT_NAME}Config.cmake"
            "configs/${PROJECT_NAME}Functions.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION
            ${package_location}
    )
endif()

if (${build_network_e2e})
    add_subdirectory(network_e2e/tests)
endif()
