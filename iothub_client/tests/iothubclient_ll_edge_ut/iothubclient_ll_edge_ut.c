// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#endif

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

void* my_gballoc_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umock_c_negative_tests.h"
#include "umocktypes_charptr.h"
#include "umocktypes_bool.h"
#include "umocktypes_stdint.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/env_variable.h"
#include "iothub_client_ll.h"
#include "iothub_client_private.h"
#undef ENABLE_MOCKS

#include "iothub_client_ll_edge.h"


static int bool_Compare(bool left, bool right)
{
    return left != right;
}

static void bool_ToString(char* string, size_t bufferSize, bool val)
{
    (void)bufferSize;
    (void)strcpy(string, val ? "true" : "false");
}

#ifndef __cplusplus
static int _Bool_Compare(_Bool left, _Bool right)
{
    return left != right;
}

static void _Bool_ToString(char* string, size_t bufferSize, _Bool val)
{
    (void)bufferSize;
    (void)strcpy(string, val ? "true" : "false");
}
#endif

#define TEST_ENV_IOTHUB_NAME "edgehubtest1"
#define TEST_ENV_IOTHUB_SUFFIX "azure-devices.net"
static const char* TEST_ENV_AUTHSCHEME = "SasToken";
static const char* TEST_ENV_DEVICEID = "Test_DeviceId";
static const char* TEST_ENV_HOSTNAME = TEST_ENV_IOTHUB_NAME "." TEST_ENV_IOTHUB_SUFFIX;
static const char* TEST_ENV_EDGEGATEWAY = "127.0.0.1";
static const char* TEST_ENV_MODULEID = "Test_ModuleId";

static const IOTHUB_CLIENT_LL_HANDLE TEST_CLIENT_HANDLE_FROM_CONNSTR =  (IOTHUB_CLIENT_LL_HANDLE)1;
static const IOTHUB_CLIENT_LL_HANDLE TEST_CLIENT_HANDLE_FROM_CREATE_MOD_INTERNAL =  (IOTHUB_CLIENT_LL_HANDLE)2;



static const IOTHUB_CLIENT_TRANSPORT_PROVIDER TEST_TRANSPORT_PROVIDER = (IOTHUB_CLIENT_TRANSPORT_PROVIDER)0x1110;

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    (void)error_code;
    ASSERT_FAIL("umock_c reported error");
}

IOTHUB_CLIENT_LL_HANDLE test_IoTHubClient_LL_CreateForModuleInternal(const IOTHUB_CLIENT_CONFIG* config, const char* module_id)
{
    ASSERT_ARE_EQUAL_WITH_MSG(bool, true, config->protocol == TEST_TRANSPORT_PROVIDER, "Protocol to configure does not mach")
    ASSERT_ARE_EQUAL_WITH_MSG(int, 0, strcmp(TEST_ENV_DEVICEID, config->deviceId), "DeviceIds don't match");
    ASSERT_ARE_EQUAL_WITH_MSG(bool, true, config->deviceKey == NULL, "deviceKey is not NULL");
    ASSERT_ARE_EQUAL_WITH_MSG(bool, true, config->deviceSasToken == NULL, "deviceSasToken is not NULL");
    ASSERT_ARE_EQUAL_WITH_MSG(int, 0, strcmp(TEST_ENV_IOTHUB_NAME, config->iotHubName), "IoTHub names don't match");
    ASSERT_ARE_EQUAL_WITH_MSG(int, 0, strcmp(TEST_ENV_IOTHUB_SUFFIX, config->iotHubSuffix), "IoTHub suffixes don't match");
    ASSERT_ARE_EQUAL_WITH_MSG(int, 0, strcmp(TEST_ENV_EDGEGATEWAY, config->protocolGatewayHostName), "Protocol gateway hosts don't match");
    ASSERT_ARE_EQUAL_WITH_MSG(int, 0, strcmp(TEST_ENV_MODULEID, module_id), "ModuleIds don't match");
    return TEST_CLIENT_HANDLE_FROM_CREATE_MOD_INTERNAL;
}

static int test_mallocAndStrcpy_s(char** destination, const char* source)
{
    (void)source;
    size_t src_len = strlen(source);
    *destination = (char*)my_gballoc_malloc(src_len + 1);
    strcpy(*destination, source);
    return 0;
}

BEGIN_TEST_SUITE(iothubclient_ll_edge_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    umock_c_init(on_umock_c_error);
    result = umocktypes_bool_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_GLOBAL_MOCK_RETURN(environment_get_variable, "");
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(environment_get_variable, NULL);

    REGISTER_GLOBAL_MOCK_RETURN(IoTHubClient_LL_CreateFromConnectionString, (IOTHUB_CLIENT_LL_HANDLE)1);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(IoTHubClient_LL_CreateFromConnectionString, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(IoTHubClient_LL_CreateForModuleInternal, test_IoTHubClient_LL_CreateForModuleInternal);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(IoTHubClient_LL_CreateForModuleInternal, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, test_mallocAndStrcpy_s);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(mallocAndStrcpy_s, 1);

    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_TRANSPORT_PROVIDER, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_LL_HANDLE, void*);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    ;
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    ;
}

static int should_skip_index(size_t current_index, const size_t skip_array[], size_t length)
{
    int result = 0;
    for (size_t index = 0; index < length; index++)
    {
        if (current_index == skip_array[index])
        {
            result = __FAILURE__;
            break;
        }
    }
    return result;
}

// If the connection string environment variable is set, ignore all other environment variables and invoke IoTHubClient_LL_CreateFromConnectionString
TEST_FUNCTION(IoTHubClient_LL_CreateForModule_use_connection_string)
{
    STRICT_EXPECTED_CALL(environment_get_variable(IGNORED_PTR_ARG)).SetReturn("ConnString!");
    STRICT_EXPECTED_CALL(IoTHubClient_LL_CreateFromConnectionString("ConnString!", TEST_TRANSPORT_PROVIDER)).SetReturn(TEST_CLIENT_HANDLE_FROM_CONNSTR);

    IOTHUB_CLIENT_LL_HANDLE h = IoTHubClient_LL_CreateForModule(TEST_TRANSPORT_PROVIDER);
    ASSERT_ARE_EQUAL_WITH_MSG(bool, true, h == TEST_CLIENT_HANDLE_FROM_CONNSTR, "IoTHubClient_LL_CreateForModule returns wrong handle");
}

// All environment variables are specified
TEST_FUNCTION(IoTHubClient_LL_CreateForModule_success)
{
    STRICT_EXPECTED_CALL(environment_get_variable(IGNORED_PTR_ARG)).SetReturn(NULL);

    STRICT_EXPECTED_CALL(environment_get_variable(IGNORED_PTR_ARG)).SetReturn(TEST_ENV_AUTHSCHEME);
    STRICT_EXPECTED_CALL(environment_get_variable(IGNORED_PTR_ARG)).SetReturn(TEST_ENV_DEVICEID);
    STRICT_EXPECTED_CALL(environment_get_variable(IGNORED_PTR_ARG)).SetReturn(TEST_ENV_HOSTNAME);
    STRICT_EXPECTED_CALL(environment_get_variable(IGNORED_PTR_ARG)).SetReturn(TEST_ENV_EDGEGATEWAY);
    STRICT_EXPECTED_CALL(environment_get_variable(IGNORED_PTR_ARG)).SetReturn(TEST_ENV_MODULEID);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
         
    STRICT_EXPECTED_CALL(IoTHubClient_LL_CreateForModuleInternal(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    IOTHUB_CLIENT_LL_HANDLE h = IoTHubClient_LL_CreateForModule(TEST_TRANSPORT_PROVIDER);
    ASSERT_ARE_EQUAL_WITH_MSG(bool, true, h == TEST_CLIENT_HANDLE_FROM_CREATE_MOD_INTERNAL, "IoTHubClient_LL_CreateForModule returns wrong handle");
}




END_TEST_SUITE(iothubclient_ll_edge_ut)
