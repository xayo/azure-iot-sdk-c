// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "iothub_client_private.h"

IOTHUB_CLIENT_LL_HANDLE IoTHubClient_CreateForModule(IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol)
{
    return IoTHubClient_CreateForModulePrivate(protocol);
}

