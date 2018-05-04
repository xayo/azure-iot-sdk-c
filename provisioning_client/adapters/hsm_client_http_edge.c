// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdbool.h>
#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/httpapiex.h"
#include "azure_c_shared_utility/env_variable.h"

#include "parson.h"

static const char* HTTP_HEADER_KEY_CONTENT_TYPE = "Content-Type";
static const char* HTTP_HEADER_VAL_CONTENT_TYPE = "application/json; charset=utf-8";
static const char* HSM_EDGE_SIGN_JSON_KEY_ID = "KeyId";
static const char* HSM_EDGE_SIGN_JSON_ALGORITHM = "Algo";
static const char* HSM_EDGE_SIGN_DEFAULT_ALGORITHM = "HMACSHA256";
static const char* HSM_EDGE_SIGN_JSON_DATA = "Data";
static const char* HSM_EDGE_SIGN_JSON_DIGEST = "digest";


/*
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/sastoken.h"
#include "azure_c_shared_utility/base64.h"
#include "azure_c_shared_utility/sha.h"
*/

#include "hsm_client_http_edge.h"

static const char* iotedge_uri_environment = "IOTEDGE_IOTEDGEDURI";
static const char* iotedge_api_version_environment = "IOTEDGE_IOTEDGEDVERSION";

typedef struct HSM_CLIENT_HTTP_EDGE
{
    char* iotedge_uri; // IOTEDGE_IOTEDGEDURI
    char* api_version; // IOTEDGE_IOTEDGEDVERSION
} HSM_CLIENT_HTTP_EDGE;


static HSM_CLIENT_HTTP_EDGE_INTERFACE http_edge_interface = 
{

    hsm_client_http_edge_create,
    hsm_client_http_edge_destroy,
    hsm_client_http_edge_sign_data
};

static int initialize_http_edge_device(HSM_CLIENT_HTTP_EDGE* http_edge)
{
    int result;
    const char* iotedge_uri;
    const char* api_version;

    if ((iotedge_uri = environment_get_variable(iotedge_uri_environment)) == NULL)
    {
        LogError("Environment variable %s not specified", iotedge_uri_environment);
        result = __FAILURE__;
    }
    else if ((api_version = environment_get_variable(iotedge_api_version_environment)) == NULL)
    {
        LogError("Environment variable %s not specified", iotedge_api_version_environment);
        result = __FAILURE__;
    }
    else if (mallocAndStrcpy_s(&http_edge->iotedge_uri, iotedge_uri) != 0)
    {
        LogError("Failed copying data");
        result = __FAILURE__;
    }
    else if (mallocAndStrcpy_s(&http_edge->api_version, api_version) != 0)
    {
        LogError("Failed copying data");
        result = __FAILURE__;
    }
    else
    {
        result = 0;
    }

    return result;
}


HSM_CLIENT_HANDLE hsm_client_http_edge_create()
{
    HSM_CLIENT_HTTP_EDGE* result;
    result = malloc(sizeof(HSM_CLIENT_HTTP_EDGE));
    if (result == NULL)
    {
        LogError("Failure: malloc HSM_CLIENT_HTTP_EDGE.");
    }
    else
    {
        memset(result, 0, sizeof(HSM_CLIENT_HTTP_EDGE));
        if (initialize_http_edge_device(result) != 0)
        {
            LogError("Failure initializing http edge device.");
            free(result);
            result = NULL;
        }
    }
    return (HSM_CLIENT_HANDLE)result;
}

void hsm_client_http_edge_destroy(HSM_CLIENT_HANDLE handle)
{
    if (handle != NULL)
    {
        HSM_CLIENT_HTTP_EDGE* hsm_client_info = (HSM_CLIENT_HTTP_EDGE*)handle;
        free(hsm_client_info);
    }
}

int hsm_client_http_edge_init(void)
{
    return 0;
}

void hsm_client_http_edge_deinit(void)
{
}

const HSM_CLIENT_HTTP_EDGE_INTERFACE* hsm_client_http_edge_interface()
{
    return &http_edge_interface;
}

static BUFFER_HANDLE construct_json_signing_blob(const unsigned char* data, const char* module_id)
{
    JSON_Value* root_value = NULL;
    JSON_Object* root_object = NULL;
    BUFFER_HANDLE result = NULL;
    char* serialized_string = NULL;

    if ((root_value = json_value_init_object()) == NULL)
    {
        LogError("json_value_init_object failed");
        result = NULL;
    }
    else if ((root_object = json_value_get_object(root_value)) == NULL)
    {
        LogError("json_value_get_object failed");
        result = NULL;
    }
    else if ((json_object_set_string(root_object, HSM_EDGE_SIGN_JSON_KEY_ID, module_id)) != JSONSuccess)
    {
        LogError("json_object_set_string failed for keyId");
        result = NULL;
    }
    else if ((json_object_set_string(root_object, HSM_EDGE_SIGN_JSON_ALGORITHM, HSM_EDGE_SIGN_DEFAULT_ALGORITHM)) != JSONSuccess)
    {
        LogError("json_object_set_string failed for algorithm");
        result = NULL;
    }
    else if ((json_object_set_string(root_object, HSM_EDGE_SIGN_JSON_DATA, (const char*)data)) != JSONSuccess)
    {
        LogError("json_object_set_string failed for data");
        result = NULL;
    }
    else if ((serialized_string = json_serialize_to_string(root_value)) == NULL)
    {
        LogError("json_serialize_to_string failed");
        result = NULL;
    }
    else if ((result = BUFFER_create((const unsigned char*)serialized_string, strlen(serialized_string))) == NULL)
    {
        LogError("Buffer_Create failed");
        result = NULL;
    }

    json_free_serialized_string(serialized_string);
    json_object_clear(root_object);
    return result;
}


static BUFFER_HANDLE send_http_signing_request(HSM_CLIENT_HTTP_EDGE* hsm_client_info, BUFFER_HANDLE json_to_send)
{
    BUFFER_HANDLE http_response;
    STRING_HANDLE uri_path = NULL;
    HTTPAPIEX_HANDLE httpapi_ex_handle = NULL;
    HTTP_HEADERS_HANDLE http_headers_handle = NULL;
    unsigned int http_status;
    HTTP_HEADERS_RESULT http_headers_result;
    HTTPAPIEX_RESULT httpapiex_result;

    if ((uri_path = STRING_construct_sprintf("%s/modules/%s/certificate/server?api-version=%s", hsm_client_info->iotedge_uri, moduleId, hsm_client_info->api_version)) == NULL)
    {
        LogError("STRING_construct_sprintffailed");
        http_response = NULL;
    }        
    else if ((httpapi_ex_handle = HTTPAPIEX_Create(hostname)) == NULL)
    {
        LogError("HTTPAPIEX_Create failed");
        http_response = NULL;
    }
    else if ((http_headers_handle = HTTPHeaders_Alloc()) == NULL)
    {
        LogError("HTTPAPIEX_Create failed");
        http_response = NULL;
    }
    else if ((http_headers_result = HTTPHeaders_AddHeaderNameValuePair(http_headers_handle, HTTP_HEADER_KEY_CONTENT_TYPE, HTTP_HEADER_VAL_CONTENT_TYPE)) != HTTP_HEADERS_OK)
    {
        LogError("HTTPHeaders_AddHeaderNameValuePair failed, error=%d", http_headers_result);
        http_response = NULL;
    }
    else if ((httpapiex_result = HTTPAPIEX_ExecuteRequest(httpapi_ex_handle, HTTPAPI_REQUEST_POST, STRING_c_str(uri_path), NULL,
                                                           json_to_send, &http_status, NULL, http_response)) != HTTPAPIEX_OK)
    {
        LogError("HTTPAPIEX_ExecuteRequest failed, error=%d", httpapiex_result);
        http_response = NULL;        
    }
    else if (http_status >= 300)
    {
        LogError("executing HTTP request fails, http_status=%d, response_buffer=%s", http_status, (const char*)BUFFER_u_char(http_response));
        BUFFER_delete(http_response);
        http_response = NULL;        
    }

    HTTPAPIEX_Destroy(httpapi_ex_handle);
    HTTPHeaders_Free(http_headers_handle);
    STRING_delete(uri_path);
    return http_response;
}


static int parse_json_signing_response(BUFFER_HANDLE http_response, unsigned char** signed_value, size_t* signed_len)
{
    int result;
    const char* http_response_str;
    JSON_Value* root_value = NULL;
    JSON_Object* root_object = NULL;
    const char* digest;

    if ((http_response_str = (const char*)BUFFER_u_char(http_response)) == NULL)
    {
        LogError("BUFFER_u_char reading http_response");
        result = __FAILURE__;
    }
    else if ((root_value = json_parse_string(http_response_str)) == NULL)
    {
        LogError("json_parse_string failed");
        result = __FAILURE__;
    }
    else if ((root_object = json_value_get_object(root_value)) == NULL)
    {
        LogError("json_value_get_object failed");
        result = __FAILURE__;
    }
    else if ((digest = json_object_dotget_string(root_object, HSM_EDGE_SIGN_JSON_DIGEST)) == NULL)
    {
        LogError("json_value_get_object failed to get %s", HSM_EDGE_SIGN_JSON_DIGEST);
        result = __FAILURE__;
    }
    else if (mallocAndStrcpy_s((char**)signed_value, digest) != 0)
    {
        LogError("Allocating signed_value failed");
        result = __FAILURE__;
    }
    else
    {     
        *signed_len = strlen(digest);
        result = 0;
    }

    json_object_clear(root_object);
    return result;
}

int hsm_client_http_edge_sign_data(HSM_CLIENT_HANDLE handle, const unsigned char* data, size_t data_len, unsigned char** signed_value, size_t* signed_len)
{
    int result;
    BUFFER_HANDLE json_to_send = NULL;
    BUFFER_HANDLE http_response = NULL;
    
    if (handle == NULL || data == NULL || data_len == 0 || signed_value == NULL || signed_len == NULL)
    {
        LogError("Invalid handle value specified handle: %p, data: %p, data_len: %zu, signed_value: %p, signed_len: %p", handle, data, data_len, signed_value, signed_len);
        result = __FAILURE__;
    }
    else
    {
        HSM_CLIENT_HTTP_EDGE* hsm_client_info = (HSM_CLIENT_HTTP_EDGE*)handle;

        if ((json_to_send = construct_json_signing_blob(data, NULL)) == NULL)
        {
            LogError("construct_json_signing_blob failed");
            result = __FAILURE__;
        }
        else if ((http_response = send_http_signing_request(hsm_client_info, json_to_send)) == NULL)
        {
            LogError("send_http_signing_request failed");
            result = __FAILURE__;
        }
        else if (parse_json_signing_response(http_response, signed_value, signed_len) == 0)
        {
            LogError("parse_json_signing_response failed");
            result = __FAILURE__;
        }
        else
        {
            result = 0;
        }
    }

    BUFFER_delete(json_to_send);
    BUFFER_delete(http_response);
    return result;
}

