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

#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/gballoc.h"
// #include "azure_c_shared_utility/doublylinkedlist.h"
// #include "azure_c_shared_utility/singlylinkedlist.h"
// #include "azure_c_shared_utility/string_tokenizer.h"
// #include "azure_c_shared_utility/strings.h"
// #include "azure_c_shared_utility/tickcounter.h"
// #include "azure_c_shared_utility/agenttime.h"
// #include "azure_c_shared_utility/constbuffer.h"
// #include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/env_variable.h"


// TODO: Optimize this list for minimal number of includes.
// #include "iothub_client_version.h"
// #include "iothub_message.h"
// #include "iothub_client_authorization.h"
// #include "iothub_client_diagnostic.h"
#include "iothub_client_ll.h"
#include "iothub_client_private.h"


#undef ENABLE_MOCKS

#include "iothub_client_ll.h"
#ifdef USE_PROV_MODULE
#include "iothub_client_hsm_ll.h"
#endif

#include "iothub_client_options.h"

#define ENABLE_MOCKS

#ifndef DONT_USE_UPLOADTOBLOB
#include "iothub_client_ll_uploadtoblob.h"
#endif

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

#undef ENABLE_MOCKS

#include "iothub_client_ll_edge.h"

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    (void)error_code;
    ASSERT_FAIL("umock_c reported error");
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

    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_TRANSPORT_PROVIDER, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_LL_HANDLE, void*);


#if 0
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(STRING_TOKENIZER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_LL_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_CONFIRMATION_RESULT, int);
    REGISTER_UMOCK_ALIAS_TYPE(TICK_COUNTER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(PDLIST_ENTRY, void*);
    REGISTER_UMOCK_ALIAS_TYPE(TRANSPORT_LL_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_TRANSPORT_PROVIDER, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_DEVICE_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_MESSAGE_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_DEVICE_TWIN_STATE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(CONSTBUFFER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_IDENTITY_TYPE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(BUFFER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(METHOD_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_AUTHORIZATION_HANDLE, void*);

    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_RESULT, int);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUBMESSAGE_DISPOSITION_RESULT, int);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_PROCESS_ITEM_RESULT, int);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_STATUS, int);
    REGISTER_UMOCK_ALIAS_TYPE(DEVICE_TWIN_UPDATE_STATE, int);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_CONNECTION_STATUS, int);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_CONNECTION_STATUS_REASON, int);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_RETRY_POLICY, int);
    REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_MATCH_FUNCTION, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ACTION_FUNCTION, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_CONDITION_FUNCTION, void*);

#ifndef DONT_USE_UPLOADTOBLOB
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_LL_UPLOADTOBLOB_HANDLE, void*);
#endif // DONT_USE_UPLOADTOBLOB

    REGISTER_GLOBAL_MOCK_RETURN(IoTHubClient_GetVersionString, "version 1.0");

    REGISTER_GLOBAL_MOCK_RETURN(FAKE_IoTHubTransport_Subscribe_DeviceTwin, 0);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(FAKE_IoTHubTransport_Subscribe_DeviceTwin, __FAILURE__);

    REGISTER_GLOBAL_MOCK_FAIL_RETURN(FAKE_IoTHubTransport_ProcessItem, IOTHUB_PROCESS_OK);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(FAKE_IoTHubTransport_ProcessItem, IOTHUB_PROCESS_ERROR);

    REGISTER_GLOBAL_MOCK_HOOK(FAKE_IoTHubTransport_GetHostname, my_FAKE_IoTHubTransport_GetHostname);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(FAKE_IoTHubTransport_GetHostname, NULL);
    REGISTER_GLOBAL_MOCK_RETURN(FAKE_IoTHubTransport_SetOption, IOTHUB_CLIENT_OK);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(FAKE_IoTHubTransport_SetOption, IOTHUB_CLIENT_ERROR);
    REGISTER_GLOBAL_MOCK_RETURN(FAKE_IoTHubTransport_Create, TEST_TRANSPORT_LL_HANDLE);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(FAKE_IoTHubTransport_Create, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(FAKE_IoTHubTransport_Register, my_FAKE_IoTHubTransport_Register);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(FAKE_IoTHubTransport_Register, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(FAKE_IoTHubTransport_Unregister, my_FAKE_IoTHubTransport_Unregister);
    REGISTER_GLOBAL_MOCK_RETURN(FAKE_IoTHubTransport_Subscribe, 0);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(FAKE_IoTHubTransport_Subscribe, __FAILURE__);
    REGISTER_GLOBAL_MOCK_HOOK(FAKE_IoTHubTransport_SetRetryPolicy, my_FAKE_IoTHubTransport_SetRetryPolicy);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(FAKE_IoTHubTransport_SetRetryPolicy, __FAILURE__);
    REGISTER_GLOBAL_MOCK_HOOK(FAKE_IoTHubTransport_GetSendStatus, my_FAKE_IoTHubTransport_GetSendStatus);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(FAKE_IoTHubTransport_GetSendStatus, IOTHUB_CLIENT_ERROR);
    REGISTER_GLOBAL_MOCK_RETURN(FAKE_IoTHubTransport_Subscribe_DeviceMethod, 0);

    REGISTER_GLOBAL_MOCK_HOOK(FAKE_IotHubTransport_Subscribe_InputQueue, my_FAKE_IoTHubTransport_Common_Subscribe_InputQueue);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(FAKE_IotHubTransport_Subscribe_InputQueue, IOTHUB_CLIENT_ERROR);

    REGISTER_GLOBAL_MOCK_HOOK(FAKE_IotHubTransport_Unsubscribe_InputQueue, my_FAKE_IoTHubTransport_Common_Unsubscribe_InputQueue);

    REGISTER_GLOBAL_MOCK_FAIL_RETURN(FAKE_IoTHubTransport_Subscribe_DeviceMethod, __FAILURE__);
    REGISTER_GLOBAL_MOCK_RETURN(FAKE_IoTHubMessage_GetMessageId, "1");
    REGISTER_GLOBAL_MOCK_RETURN(FAKE_IoTHubTransport_SendMessageDisposition, IOTHUB_CLIENT_OK);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(FAKE_IoTHubTransport_SendMessageDisposition, IOTHUB_CLIENT_ERROR);

    REGISTER_GLOBAL_MOCK_HOOK(FAKE_IoTHubTransport_DeviceMethod_Response, my_FAKE_DeviceMethod_Response);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(FAKE_IoTHubTransport_DeviceMethod_Response, __FAILURE__);

#ifndef DONT_USE_UPLOADTOBLOB
    REGISTER_GLOBAL_MOCK_HOOK(IoTHubClient_LL_UploadToBlob_Create, my_IoTHubClient_LL_UploadToBlob_Create);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(IoTHubClient_LL_UploadToBlob_Create, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(IoTHubClient_LL_UploadToBlob_Destroy, my_IoTHubClient_LL_UploadToBlob_Destroy);
    REGISTER_GLOBAL_MOCK_RETURN(IoTHubClient_LL_UploadToBlob_SetOption, IOTHUB_CLIENT_OK);
#endif

    REGISTER_GLOBAL_MOCK_RETURN(deviceMethodCallback, 200);

    REGISTER_GLOBAL_MOCK_RETURN(IoTHubMessage_CreateFromString, (IOTHUB_MESSAGE_HANDLE)0x44);
    REGISTER_GLOBAL_MOCK_RETURN(IoTHubMessage_Clone, (IOTHUB_MESSAGE_HANDLE)0x44);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(IoTHubMessage_Clone, NULL);

    REGISTER_GLOBAL_MOCK_RETURN(IoTHubMessage_SetOutputName, IOTHUB_MESSAGE_OK);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(IoTHubMessage_SetOutputName, IOTHUB_MESSAGE_ERROR);

    REGISTER_GLOBAL_MOCK_RETURN(IoTHubMessage_GetInputName, TEST_INPUT_NAME);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(IoTHubMessage_GetInputName, NULL);

    REGISTER_GLOBAL_MOCK_RETURN(IoTHubClient_Diagnostic_AddIfNecessary, 0);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(IoTHubClient_Diagnostic_AddIfNecessary, 100);

    REGISTER_GLOBAL_MOCK_HOOK(IoTHubClient_Auth_CreateFromDeviceAuth, my_IoTHubClient_Auth_CreateFromDeviceAuth);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(IoTHubClient_Auth_CreateFromDeviceAuth, NULL);

    REGISTER_GLOBAL_MOCK_RETURN(get_time, (time_t)TEST_TIME_VALUE);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

    REGISTER_GLOBAL_MOCK_HOOK(STRING_new, my_STRING_new);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_new, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(STRING_construct, my_STRING_construct);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_construct, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(STRING_concat_with_STRING, m_STRING_concat_with_STRING);
    REGISTER_GLOBAL_MOCK_RETURN(STRING_c_str, TEST_STRING_VALUE);
    REGISTER_GLOBAL_MOCK_HOOK(STRING_clone, my_STRING_clone);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_clone, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(STRING_delete, my_STRING_delete);

    REGISTER_GLOBAL_MOCK_RETURN(BUFFER_build, 0);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(BUFFER_build, __FAILURE__);

    REGISTER_GLOBAL_MOCK_HOOK(CONSTBUFFER_Create, my_CONSTBUFFER_Create);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(CONSTBUFFER_Create, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(CONSTBUFFER_Destroy, my_CONSTBUFFER_Destroy);

    REGISTER_GLOBAL_MOCK_HOOK(STRING_TOKENIZER_create, my_STRING_TOKENIZER_create);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_TOKENIZER_create, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(STRING_TOKENIZER_get_next_token, my_STRING_TOKENIZER_get_next_token);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_TOKENIZER_get_next_token, __FAILURE__);
    REGISTER_GLOBAL_MOCK_HOOK(STRING_TOKENIZER_destroy, my_STRING_TOKENIZER_destroy);

    REGISTER_GLOBAL_MOCK_HOOK(tickcounter_create, my_tickcounter_create);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(tickcounter_create, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(tickcounter_destroy, my_tickcounter_destroy);

    REGISTER_GLOBAL_MOCK_HOOK(tickcounter_get_current_ms, my_tickcounter_get_current_ms);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(tickcounter_get_current_ms, __FAILURE__);

    REGISTER_GLOBAL_MOCK_HOOK(DList_InitializeListHead, real_DList_InitializeListHead);
    REGISTER_GLOBAL_MOCK_HOOK(DList_IsListEmpty, real_DList_IsListEmpty);
    REGISTER_GLOBAL_MOCK_HOOK(DList_InsertTailList, real_DList_InsertTailList);
    REGISTER_GLOBAL_MOCK_HOOK(DList_InsertHeadList, real_DList_InsertHeadList);
    REGISTER_GLOBAL_MOCK_HOOK(DList_AppendTailList, real_DList_AppendTailList);
    REGISTER_GLOBAL_MOCK_HOOK(DList_RemoveEntryList, real_DList_RemoveEntryList);
    REGISTER_GLOBAL_MOCK_HOOK(DList_RemoveHeadList, real_DList_RemoveHeadList);

    REGISTER_GLOBAL_MOCK_RETURN(test_message_callback_async, IOTHUBMESSAGE_ACCEPTED);
    REGISTER_GLOBAL_MOCK_RETURN(messageCallback, IOTHUBMESSAGE_ACCEPTED);
    REGISTER_GLOBAL_MOCK_RETURN(messageCallbackEx, true);
    REGISTER_GLOBAL_MOCK_HOOK(messageInputCallbackEx, real_messageInputCallbackEx);

    REGISTER_GLOBAL_MOCK_HOOK(IoTHubClient_Auth_Create, my_IoTHubClient_Auth_Create);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(IoTHubClient_Auth_Create, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(IoTHubClient_Auth_Destroy, my_IoTHubClient_Auth_Destroy);

    REGISTER_GLOBAL_MOCK_HOOK(platform_get_platform_info, my_plafrom_get_platform_info);

    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_create, real_singlylinkedlist_create);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_destroy, real_singlylinkedlist_destroy);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_add, real_singlylinkedlist_add);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_head_item, real_singlylinkedlist_get_head_item);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove, real_singlylinkedlist_remove);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_item_get_value, real_singlylinkedlist_item_get_value);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_next_item, real_singlylinkedlist_get_next_item);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_find, real_singlylinkedlist_find);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_foreach, real_singlylinkedlist_foreach);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove_if, real_singlylinkedlist_remove_if);
#endif
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
}

TEST_FUNCTION_INITIALIZE(method_init)
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

static const IOTHUB_CLIENT_TRANSPORT_PROVIDER TEST_TRANSPORT_PROVIDER = (IOTHUB_CLIENT_TRANSPORT_PROVIDER)0x1110;

TEST_FUNCTION(test_1)
{
    STRICT_EXPECTED_CALL(environment_get_variable(IGNORED_PTR_ARG)).SetReturn("ConnString!");
    STRICT_EXPECTED_CALL(IoTHubClient_LL_CreateFromConnectionString("ConnString!", TEST_TRANSPORT_PROVIDER)).SetReturn((IOTHUB_CLIENT_LL_HANDLE)1);

    IOTHUB_CLIENT_LL_HANDLE h = Iothub_LL_Create_For_Module(TEST_TRANSPORT_PROVIDER);
    (void)h;
}



END_TEST_SUITE(iothubclient_ll_edge_ut)
