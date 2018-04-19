// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PROV_TRANSPORT_AMQP_CLIENT_H
#define PROV_TRANSPORT_AMQP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#include <cstdint>
#include <cstddef>
#else
#include <stdint.h>
#include <stddef.h>
#endif /* __cplusplus */

#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/macro_utils.h"
#include "azure_prov_client/internal/prov_transport.h"

const PROV_DEVICE_TRANSPORT_PROVIDER* Prov_Device_AMQP_Protocol(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // PROV_TRANSPORT_AMQP_CLIENT_H
