// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdbool.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/buffer_.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/base64.h"
#include "azure_uhttp_c/uhttp.h"

#include "azure_c_shared_utility/envvariable.h"

#include "parson.h"

static const char* HTTP_HEADER_KEY_CONTENT_TYPE = "Content-Type";
static const char* HTTP_HEADER_VAL_CONTENT_TYPE = "application/json; charset=utf-8";
static const char* HSM_EDGE_SIGN_JSON_KEY_ID = "keyId";
static const char* HSM_EDGE_SIGN_JSON_KEY_ID_VALUE = "primary";
static const char* HSM_EDGE_SIGN_JSON_ALGORITHM = "algo";
static const char* HSM_EDGE_SIGN_JSON_ALGORITHM_VALUE = "HMACSHA256";
static const char* HSM_EDGE_SIGN_JSON_DATA = "data";
static const char* HSM_EDGE_SIGN_JSON_DIGEST = "digest";

#include "hsm_client_http_edge.h"

static const char* ENVIRONMENT_VAR_EDGEURI = "IOTEDGE_IOTEDGEDURI";
static const char* ENVIRONMENT_VAR_EDGEVERSION = "IOTEDGE_IOTEDGEDVERSION";
static const char* ENVIRONMENT_VAR_EDGEMODULEID = "IOTEDGE_MODULEID";


typedef struct HSM_CLIENT_HTTP_EDGE
{
    char* edge_hostname;
    int   edge_portnumber;
    char* api_version; // IOTEDGE_IOTEDGEDVERSION
    char* module_id;
} HSM_CLIENT_HTTP_EDGE;

static const char* http_prefix = "http://";
static const int http_prefix_len = sizeof(http_prefix) - 1;

static HSM_CLIENT_HTTP_EDGE_INTERFACE http_edge_interface = 
{

    hsm_client_http_edge_create,
    hsm_client_http_edge_destroy,
    hsm_client_http_edge_sign_data
};

// This is the string we use to connect to the edge device itself.  An example will be 
// http://127.0.0.1:8080.  Note NOT "https" as that would require us to trust the edgelet's
// server certificate, which we can't because we're still bootstrapping.
static int read_and_parse_edge_uri(HSM_CLIENT_HTTP_EDGE* hsm_client_http_edge)
{
    const char* iotedge_uri;
    const char* colon_begin;
    int result;

    if ((iotedge_uri = environment_get_variable(ENVIRONMENT_VAR_EDGEURI)) == NULL)
    {
        LogError("Environment variable %s not specified", ENVIRONMENT_VAR_EDGEURI);
        result = __FAILURE__;
    }
    else if (strncmp(iotedge_uri, http_prefix, http_prefix_len) != 0)
    {
        LogError("EdgeUri is set to %s, but must begin with %s", iotedge_uri, http_prefix);
        result = __FAILURE__;
    }
    else if ((colon_begin = strchr(iotedge_uri + http_prefix_len + 1, ':')) == NULL)
    {
        LogError("EdgeUri is set to %s, missing ':' to indicate port number", iotedge_uri);
        result = __FAILURE__;
    }
    else if ((hsm_client_http_edge->edge_portnumber = atoi(colon_begin + 1)) == 0)
    {
        LogError("EdgeUri is set to %s, port number is not legal", iotedge_uri);
        result = __FAILURE__;
    }
    else if ((hsm_client_http_edge->edge_hostname = malloc(colon_begin - iotedge_uri)) == NULL)
    {
        LogError("Failed allocating edge_hostname");
        result = __FAILURE__;
    }
    else
    {
        const char* hostname_start = iotedge_uri + http_prefix_len;
        size_t chars_to_copy = colon_begin - iotedge_uri - http_prefix_len;
        strncpy(hsm_client_http_edge->edge_hostname, hostname_start, chars_to_copy);
        hsm_client_http_edge->edge_hostname[chars_to_copy] = 0;
        result = 0;
    }

    return result;
}

static int initialize_http_edge_device(HSM_CLIENT_HTTP_EDGE* hsm_client_http_edge)
{
    int result;
    const char* api_version;
    const char* module_id;

    if ((api_version = environment_get_variable(ENVIRONMENT_VAR_EDGEVERSION)) == NULL)
    {
        LogError("Environment variable %s not specified", ENVIRONMENT_VAR_EDGEVERSION);
        result = __FAILURE__;
    }
    else if ((module_id = environment_get_variable(ENVIRONMENT_VAR_EDGEMODULEID)) == NULL)
    {
        LogError("Environment variable %s not specified", ENVIRONMENT_VAR_EDGEMODULEID);
        result = __FAILURE__;
    }
    else if (read_and_parse_edge_uri(hsm_client_http_edge) != 0)
    {
        LogError("read_and_parse_edge_uri failed");
        result = __FAILURE__;
    }
    else if (mallocAndStrcpy_s(&hsm_client_http_edge->api_version, api_version) != 0)
    {
        LogError("Failed copying api_version");
        result = __FAILURE__;
    }
    else if (mallocAndStrcpy_s(&hsm_client_http_edge->module_id, module_id) != 0)
    {
        LogError("Failed copying module_id");
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
        HSM_CLIENT_HTTP_EDGE* hsm_client_http_edge = (HSM_CLIENT_HTTP_EDGE*)handle;
        free(hsm_client_http_edge->edge_hostname);
        free(hsm_client_http_edge->api_version);
        free(hsm_client_http_edge->module_id);
        free(hsm_client_http_edge);
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

static BUFFER_HANDLE construct_json_signing_blob(const unsigned char* data)
{
    JSON_Value* root_value = NULL;
    JSON_Object* root_object = NULL;
    BUFFER_HANDLE result = NULL;
    char* serialized_string = NULL;
    STRING_HANDLE data_string = NULL;
    STRING_HANDLE data_url_encoded = NULL;
    STRING_HANDLE data_base64_encoded = NULL;
    
    // The caller passes us the data in the form "<tokenScope>\n<expire_time>".  We need to 
    // URL encode the tokenScope for HTTP transmission, but not the \n, so build up string appropriately.
    const char* carriage_return_in_data = strchr((const char*)data, '\n');
    if ((carriage_return_in_data == NULL) || (*(carriage_return_in_data+1) == 0))
    {
        LogError("No carriage return in data", data);
        result = NULL;
    }
    else if ((data_string = STRING_construct_n((const char*)data, carriage_return_in_data - (const char*)data)) == NULL)
    {
        LogError("creating data string failed");
        result = NULL;
    }    
    else if ((data_url_encoded = URL_Encode(data_string)) == NULL)
    {
        LogError("url encoding of string %s failed", data);
        result = NULL;
    }
    else if ((STRING_concat(data_url_encoded, carriage_return_in_data)) != 0)
    {
        LogError("STRING_concat failed");
        result = NULL;
    }
    else if ((data_base64_encoded = Base64_Encode_Bytes((const unsigned char*)STRING_c_str(data_url_encoded), strlen(STRING_c_str(data_url_encoded)))) == NULL)
    {
        LogError("base64 encoding of string %s failed", STRING_c_str(data_url_encoded));
        result = NULL;
    }
    else if ((root_value = json_value_init_object()) == NULL)
    {
        LogError("json_value_init_object failed");
        result = NULL;
    }
    else if ((root_object = json_value_get_object(root_value)) == NULL)
    {
        LogError("json_value_get_object failed");
        result = NULL;
    }
    else if ((json_object_set_string(root_object, HSM_EDGE_SIGN_JSON_KEY_ID, HSM_EDGE_SIGN_JSON_KEY_ID_VALUE)) != JSONSuccess)
    {
        LogError("json_object_set_string failed for keyId");
        result = NULL;
    }
    else if ((json_object_set_string(root_object, HSM_EDGE_SIGN_JSON_ALGORITHM, HSM_EDGE_SIGN_JSON_ALGORITHM_VALUE)) != JSONSuccess)
    {
        LogError("json_object_set_string failed for algorithm");
        result = NULL;
    }
    else if ((json_object_set_string(root_object, HSM_EDGE_SIGN_JSON_DATA, STRING_c_str(data_base64_encoded))) != JSONSuccess)
    {
        LogError("json_object_set_string failed for data");
        result = NULL;
    }
    else if ((serialized_string = json_serialize_to_string(root_value)) == NULL)
    {
        LogError("json_serialize_to_string failed");
        result = NULL;
    }
    else if ((result = BUFFER_create((const unsigned char*)serialized_string, strlen(serialized_string) + 1)) == NULL)
    {
        LogError("Buffer_Create failed");
        result = NULL;
    }

    json_free_serialized_string(serialized_string);
    json_object_clear(root_object);
    STRING_delete(data_base64_encoded);
    STRING_delete(data_url_encoded);
    STRING_delete(data_string);
    return result;
}

static void on_edge_hsm_http_error(void* callback_ctx, HTTP_CALLBACK_REASON error_result)
{
    HSM_HTTP_EDGE_SIGNING_CONTEXT* sign_context = (HSM_HTTP_EDGE_SIGNING_CONTEXT*)callback_ctx;
    if (sign_context == NULL)
    {
        LogError("on_edge_hsm_http_error invoked with invalid context.  reason=%d", error_result);
    }
    else
    {
        LogError("on_edge_hsm_http_error invoked.  reason=%d", error_result);
        sign_context->continue_running = false;
    }
}

static void on_edge_hsm_http_recv(void* callback_ctx, HTTP_CALLBACK_REASON request_result, const unsigned char* content, size_t content_length, unsigned int status_code, HTTP_HEADERS_HANDLE response_headers)
{
    (void)response_headers;
    HSM_HTTP_EDGE_SIGNING_CONTEXT* sign_context = (HSM_HTTP_EDGE_SIGNING_CONTEXT*)callback_ctx;
    if (sign_context == NULL)
    {
        LogError("on_edge_hsm_http_recv invoked with invalid context");
    }
    else
    {
        if (request_result != HTTP_CALLBACK_REASON_OK)
        {
            LogError("on_edge_hsm_http_recv request result = %d", request_result);
        }
        else if (status_code >= 300)
        {
            LogError("executing HTTP request fails, status=%d, response_buffer=%s", status_code, content ? (const char*)content : "unspecified");
        }
        else if (content == NULL)
        {
            LogError("executing HTTP request fails, content not set");
        }
        else if ((sign_context->http_response = BUFFER_create(content, content_length)) == NULL)
        {
            LogError("failed copying response buffer");
        }
        else
        {
            ; // success
        }
   
        sign_context->continue_running = false;
    }
}

static void on_edge_hsm_http_connected(void* callback_ctx, HTTP_CALLBACK_REASON connect_result)
{
    if (connect_result != HTTP_CALLBACK_REASON_OK)
    {
        HSM_HTTP_EDGE_SIGNING_CONTEXT* sign_context = (HSM_HTTP_EDGE_SIGNING_CONTEXT*)callback_ctx;
        if (callback_ctx == NULL)
        {
            LogError("on_http_connected reports error %d but no context", connect_result);
        }
        else
        {
            LogError("on_http_connected reports error %d", connect_result);
            sign_context->continue_running = false;
        }
    }
}

static const int HSM_HTTP_EDGE_MAXIMUM_REQUEST_TIME = 60; // 1 Minute

static int send_and_poll_http_signing_request(HTTP_CLIENT_HANDLE http_handle, HTTP_HEADERS_HANDLE http_headers_handle, const char* uri_path, BUFFER_HANDLE json_to_send, HSM_HTTP_EDGE_SIGNING_CONTEXT* sign_context)
{
    const unsigned char* json_to_send_str = BUFFER_u_char(json_to_send);
    time_t start_request_time = get_time(NULL);
    bool timed_out = false;
    HTTP_CLIENT_RESULT http_client_result;

    if ((http_client_result = uhttp_client_execute_request(http_handle, HTTP_CLIENT_REQUEST_POST, uri_path, http_headers_handle, json_to_send_str, strlen((const char*)json_to_send_str), on_edge_hsm_http_recv, sign_context)) != HTTP_CLIENT_OK)
    {
        LogError("uhttp_client_execute_request failed, result=%d", http_client_result);
    }
    else
    {
        do
        {
            uhttp_client_dowork(http_handle);
            timed_out = difftime(get_time(NULL), start_request_time) > HSM_HTTP_EDGE_MAXIMUM_REQUEST_TIME;
        } while ((sign_context->continue_running == true) && (timed_out == false));
    }

    return (sign_context->http_response != NULL) ? 0 : __FAILURE__;
}

static BUFFER_HANDLE send_http_signing_request(HSM_CLIENT_HTTP_EDGE* hsm_client_http_edge, BUFFER_HANDLE json_to_send)
{
    int result;
    STRING_HANDLE uri_path = NULL;
    HTTP_CLIENT_HANDLE http_handle = NULL;
    HTTP_HEADERS_HANDLE http_headers_handle = NULL;

    HTTP_CLIENT_RESULT http_open_result;
    HTTP_HEADERS_RESULT http_headers_result;

    SOCKETIO_CONFIG config;
    config.accepted_socket = NULL;
    config.hostname = hsm_client_http_edge->edge_hostname;
    config.port = hsm_client_http_edge->edge_portnumber;

    HSM_HTTP_EDGE_SIGNING_CONTEXT sign_context;
    sign_context.continue_running = true;
    sign_context.http_response = NULL;

    if ((uri_path = STRING_construct_sprintf("/modules/%s/sign?api-version=%s", hsm_client_http_edge->module_id, hsm_client_http_edge->api_version)) == NULL)
    {
        LogError("STRING_construct_sprintf failed");
        result = __FAILURE__;
    }
    else if ((http_handle = uhttp_client_create(socketio_get_interface_description(), &config, on_edge_hsm_http_error, &sign_context)) == NULL)
    {
        LogError("uhttp_client_create failed");
        result = __FAILURE__;
    }
    else if ((http_open_result = uhttp_client_open(http_handle, hsm_client_http_edge->edge_hostname, hsm_client_http_edge->edge_portnumber, on_edge_hsm_http_connected, &sign_context) != HTTP_CLIENT_OK) != HTTP_CLIENT_OK)
    {
        LogError("uhttp_client_open failed, err=%d", http_open_result);
        result = __FAILURE__;
    }
    else if ((http_headers_handle = HTTPHeaders_Alloc()) == NULL)
    {
        LogError("HTTPAPIEX_Create failed");
        result = __FAILURE__;
    }
    else if ((http_headers_result = HTTPHeaders_AddHeaderNameValuePair(http_headers_handle, HTTP_HEADER_KEY_CONTENT_TYPE, HTTP_HEADER_VAL_CONTENT_TYPE)) != HTTP_HEADERS_OK)
    {
        LogError("HTTPHeaders_AddHeaderNameValuePair failed, error=%d", http_headers_result);
        result = __FAILURE__;
    }
    else if (send_and_poll_http_signing_request(http_handle, http_headers_handle, STRING_c_str(uri_path), json_to_send, &sign_context) != 0)
    {
        LogError("send_and_poll_http_signing_request failed");
        result = __FAILURE__;
    }
    else
    {
        result = 0;
    }

    HTTPHeaders_Free(http_headers_handle);
    uhttp_client_destroy(http_handle);
    STRING_delete(uri_path);

    if (result != 0)
    {
        BUFFER_delete(sign_context.http_response);
        sign_context.http_response = NULL;
    }

    return sign_context.http_response;
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
        HSM_CLIENT_HTTP_EDGE* hsm_client_http_edge = (HSM_CLIENT_HTTP_EDGE*)handle;

        if ((json_to_send = construct_json_signing_blob(data)) == NULL)
        {
            LogError("construct_json_signing_blob failed");
            result = __FAILURE__;
        }
        else if ((http_response = send_http_signing_request(hsm_client_http_edge, json_to_send)) == NULL)
        {
            LogError("send_http_signing_request failed");
            result = __FAILURE__;
        }
        else if (parse_json_signing_response(http_response, signed_value, signed_len) != 0)
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

