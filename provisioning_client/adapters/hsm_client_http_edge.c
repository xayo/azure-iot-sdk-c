// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdbool.h>
#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/httpapi.h"
#include "azure_c_shared_utility/env_variable.h"


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

int hsm_client_http_edge_sign_data(HSM_CLIENT_HANDLE handle, const unsigned char* data, size_t data_len, unsigned char** signed_value, size_t* signed_len)
{
    int result;
    if (handle == NULL || data == NULL || data_len == 0 || signed_value == NULL || signed_len == NULL)
    {
        LogError("Invalid handle value specified handle: %p, data: %p, data_len: %zu, signed_value: %p, signed_len: %p", handle, data, data_len, signed_value, signed_len);
        result = __FAILURE__;
    }
    else
    {
        HSM_CLIENT_HTTP_EDGE* hsm_client_info = (HSM_CLIENT_HTTP_EDGE*)handle;
        (void)hsm_client_info;
        result = 0;
    }
    return result;
}

