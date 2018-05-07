// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstdint>
#include <cstddef>
#else
#include <stdlib.h>
#include <stdint.h>
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

#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_stdint.h"
#include "umocktypes_bool.h"
#include "umock_c_negative_tests.h"
#include "azure_c_shared_utility/macro_utils.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/buffer_.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/httpheaders.h"
#include "azure_c_shared_utility/agenttime.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_uhttp_c/uhttp.h"
#include "azure_c_shared_utility/env_variable.h"
#include "parson.h"


// #include "azure_c_shared_utility/sastoken.h"
// #include "azure_c_shared_utility/base64.h"
// #include "azure_c_shared_utility/sha.h"
// #include "azure_c_shared_utility/urlencode.h"

// #include "azure_utpm_c/tpm_codec.h"
// #include "azure_utpm_c/Marshal_fp.h"

MOCKABLE_FUNCTION(, JSON_Value*, json_parse_string, const char *, string);
MOCKABLE_FUNCTION(, char*, json_serialize_to_string, const JSON_Value*, value);
MOCKABLE_FUNCTION(, void, json_free_serialized_string, char*, string);
MOCKABLE_FUNCTION(, const char*, json_object_dotget_string, const JSON_Object*, object, const char*, name);
MOCKABLE_FUNCTION(, JSON_Status, json_object_set_string, JSON_Object*, object, const char*, name, const char*, string);
MOCKABLE_FUNCTION(, JSON_Status, json_object_clear, JSON_Object*, object);
MOCKABLE_FUNCTION(, JSON_Value*, json_value_init_object);
MOCKABLE_FUNCTION(, JSON_Object*, json_value_get_object, const JSON_Value *, value);

#undef ENABLE_MOCKS

#include "hsm_client_http_edge.h"

// TODO: Cleanup unneeded stuff
// static void my_STRING_delete(STRING_HANDLE h)
// {
//     my_gballoc_free((void*)h);
// }

static int my_mallocAndStrcpy_s(char** destination, const char* source)
{
    (void)source;
    size_t src_len = strlen(source);
    *destination = (char*)my_gballoc_malloc(src_len + 1);
    strcpy(*destination, source);
    return 0;
}

STRING_HANDLE STRING_construct_sprintf(const char* format, ...)
{
    (void)format;
    return (STRING_HANDLE)my_gballoc_malloc(1);
}


DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}


BEGIN_TEST_SUITE(hsm_client_http_edge_ut)
TEST_SUITE_INITIALIZE(suite_init)
{
    (void)umock_c_init(on_umock_c_error);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(method_cleanup)
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
            result = __LINE__;
            break;
        }
    }
    return result;
}

#if 0
TEST_FUNCTION(hsm_client_http_edge_create_succeed)
{
    //arrange
    // setup_hsm_client_tpm_create_mock();

    //act
    HSM_CLIENT_HANDLE sec_handle = hsm_client_http_edge_create();

    //assert
    ASSERT_IS_NOT_NULL(sec_handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    hsm_client_http_edge_destroy(sec_handle);
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_001: [ If any failure is encountered hsm_client_tpm_create shall return NULL ] */
TEST_FUNCTION(hsm_client_tpm_create_fail)
{
    //arrange
    setup_hsm_client_tpm_create_mock();

    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 3, 8 };

    //act
    size_t count = umock_c_negative_tests_call_count();
    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
        {
            continue;
        }

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        char tmp_msg[64];
        sprintf(tmp_msg, "secure_device_riot_create failure in test %zu/%zu", index, count);

        //act
        HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();

        //assert
        ASSERT_IS_NULL_WITH_MSG(sec_handle, tmp_msg);
    }

    //cleanup
    umock_c_negative_tests_deinit();
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_004: [ hsm_client_tpm_destroy shall free the SEC_DEVICE_INFO instance. ] */
/* Tests_SRS_SECURE_DEVICE_TPM_07_006: [ hsm_client_tpm_destroy shall free all resources allocated in this module. ]*/
TEST_FUNCTION(hsm_client_tpm_destroy_succeed)
{
    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(Deinit_TPM_Codec(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    //act
    hsm_client_tpm_destroy(sec_handle);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_005: [ if handle is NULL, hsm_client_tpm_destroy shall do nothing. ] */
TEST_FUNCTION(hsm_client_tpm_destroy_handle_NULL_succeed)
{
    //arrange
    umock_c_reset_all_calls();

    //act
    hsm_client_tpm_destroy(NULL);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_007: [ if handle or key are NULL, or key_len is 0 hsm_client_tpm_import_key shall return a non-zero value ] */
TEST_FUNCTION(hsm_client_tpm_import_key_handle_NULL_fail)
{
    //arrange

    //act
    int import_res = hsm_client_tpm_import_key(NULL, TEST_IMPORT_KEY, TEST_KEY_SIZE);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, import_res);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_007: [ if handle or key are NULL, or key_len is 0 hsm_client_tpm_import_key shall return a non-zero value ] */
TEST_FUNCTION(hsm_client_tpm_import_key_key_NULL_fail)
{
    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    //act
    int import_res = hsm_client_tpm_import_key(sec_handle, NULL, TEST_KEY_SIZE);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, import_res);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    hsm_client_tpm_destroy(sec_handle);
}

TEST_FUNCTION(hsm_client_tpm_import_key_fail)
{
    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    setup_hsm_client_tpm_import_key_mock();

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 2, 3, 4, 5, 6, 7, 10, 13 };

    //act
    size_t count = umock_c_negative_tests_call_count();
    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
        {
            continue;
        }

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        char tmp_msg[64];
        sprintf(tmp_msg, "hsm_client_tpm_import_key failure in test %zu/%zu", index, count);

        //act
        int import_res = hsm_client_tpm_import_key(sec_handle, TEST_IMPORT_KEY, TEST_KEY_SIZE);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, 0, import_res, tmp_msg);
    }
    //cleanup
    hsm_client_tpm_destroy(sec_handle);
    umock_c_negative_tests_deinit();
}

/* Tests_hsm_client_tpm_import_key shall establish a tpm session in preparation to inserting the key into the tpm. */
/* Tests_SRS_SECURE_DEVICE_TPM_07_008: [ On success hsm_client_tpm_import_key shall return zero ] */
TEST_FUNCTION(hsm_client_tpm_import_key_succeed)
{
    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    setup_hsm_client_tpm_import_key_mock();

    //act
    int import_res = hsm_client_tpm_import_key(sec_handle, TEST_IMPORT_KEY, TEST_KEY_SIZE);

    //assert
    ASSERT_ARE_EQUAL(int, 0, import_res);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    hsm_client_tpm_destroy(sec_handle);
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_013: [ If handle is NULL hsm_client_tpm_get_endorsement_key shall return NULL. ] */
TEST_FUNCTION(hsm_client_tpm_get_endorsement_key_handle_NULL_succeed)
{
    unsigned char* key;
    size_t key_len;

    //act
    int result = hsm_client_tpm_get_endorsement_key(NULL, &key, &key_len);

    //assert
    //ASSERT_IS_NULL(key);
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_027: [ If the ek_public was not initialized hsm_client_tpm_get_endorsement_key shall return NULL. ] */
TEST_FUNCTION(hsm_client_tpm_get_endorsement_key_size_0_fail)
{
    g_rsa_size = 0;
    unsigned char* key;
    size_t key_len;

    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    //act
    int result = hsm_client_tpm_get_endorsement_key(NULL, &key, &key_len);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    hsm_client_tpm_destroy(sec_handle);
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_015: [ If a failure is encountered, hsm_client_tpm_get_endorsement_key shall return NULL. ] */
TEST_FUNCTION(hsm_client_tpm_get_endorsement_key_fail)
{
    //arrange
    unsigned char* key;
    size_t key_len;

    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    setup_hsm_client_tpm_get_endorsement_key_mocks();

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 0, 2, 4 };

    //act
    size_t count = umock_c_negative_tests_call_count();
    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
        {
            continue;
        }

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        char tmp_msg[64];
        sprintf(tmp_msg, "hsm_client_tpm_get_endorsement_key failure in test %zu/%zu", index, count);

        int result = hsm_client_tpm_get_endorsement_key(NULL, &key, &key_len);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, 0, result, tmp_msg);
    }
    //cleanup
    hsm_client_tpm_destroy(sec_handle);
    umock_c_negative_tests_deinit();
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_014: [ hsm_client_tpm_get_endorsement_key shall allocate and return the Endorsement Key. ] */
TEST_FUNCTION(hsm_client_tpm_get_endorsement_key_succeed)
{
    //arrange
    unsigned char* key;
    size_t key_len;

    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    setup_hsm_client_tpm_get_endorsement_key_mocks();

    //act
    int result = hsm_client_tpm_get_endorsement_key(sec_handle, &key, &key_len);

    //assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    my_gballoc_free(key);
    hsm_client_tpm_destroy(sec_handle);
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_016: [ If handle is NULL, hsm_client_tpm_get_storage_key shall return NULL. ] */
TEST_FUNCTION(hsm_client_tpm_get_storage_key_handle_NULL_fail)
{
    //arrange
    unsigned char* key;
    size_t key_len;

    //act
    int result = hsm_client_tpm_get_storage_key(NULL, &key, &key_len);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_017: [ If the srk_public value was not initialized, hsm_client_tpm_get_storage_key shall return NULL. ] */
TEST_FUNCTION(hsm_client_tpm_get_storage_key_size_0_fail)
{
    unsigned char* key;
    size_t key_len;
    g_rsa_size = 0;

    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    //act
    int result = hsm_client_tpm_get_storage_key(NULL, &key, &key_len);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    hsm_client_tpm_destroy(sec_handle);
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_019: [ If any failure is encountered, hsm_client_tpm_get_storage_key shall return NULL. ] */
TEST_FUNCTION(hsm_client_tpm_get_storage_key_fail)
{
    unsigned char* key;
    size_t key_len;

    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    setup_hsm_client_tpm_get_storage_key_mocks();

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 0, 2, 4 };

    //act
    size_t count = umock_c_negative_tests_call_count();
    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
        {
            continue;
        }

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        char tmp_msg[64];
        sprintf(tmp_msg, "hsm_client_tpm_get_storage_key failure in test %zu/%zu", index, count);

        int result = hsm_client_tpm_get_storage_key(NULL, &key, &key_len);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, 0, result, tmp_msg);
    }

    //cleanup
    hsm_client_tpm_destroy(sec_handle);
    umock_c_negative_tests_deinit();
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_018: [ hsm_client_tpm_get_storage_key shall allocate and return the Storage Root Key. ] */
TEST_FUNCTION(hsm_client_tpm_get_storage_key_succeed)
{
    unsigned char* key;
    size_t key_len;

    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    setup_hsm_client_tpm_get_storage_key_mocks();

    //act
    int result = hsm_client_tpm_get_storage_key(sec_handle, &key, &key_len);

    //assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    my_gballoc_free(key);
    hsm_client_tpm_destroy(sec_handle);
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_020: [ If handle or data is NULL or data_len is 0, hsm_client_tpm_sign_data shall return NULL. ] */
TEST_FUNCTION(hsm_client_tpm_sign_data_handle_fail)
{
    unsigned char* key;
    size_t key_len;

    //arrange

    //act
    int result = hsm_client_tpm_sign_data(NULL, TEST_BUFFER, TEST_BUFFER_SIZE, &key, &key_len);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_020: [ If handle or data is NULL or data_len is 0, hsm_client_tpm_sign_data shall return NULL. ] */
TEST_FUNCTION(hsm_client_tpm_sign_data_data_NULL_fail)
{
    //arrange
    unsigned char* key;
    size_t key_len;

    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    //act
    int result = hsm_client_tpm_sign_data(NULL, TEST_BUFFER, TEST_BUFFER_SIZE, &key, &key_len);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    hsm_client_tpm_destroy(sec_handle);
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_020: [ If handle or data is NULL or data_len is 0, hsm_client_tpm_sign_data shall return NULL. ] */
TEST_FUNCTION(hsm_client_tpm_sign_data_size_0_fail)
{
    unsigned char* key;
    size_t key_len;

    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    //act
    int result = hsm_client_tpm_sign_data(NULL, TEST_BUFFER, TEST_BUFFER_SIZE, &key, &key_len);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    hsm_client_tpm_destroy(sec_handle);
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_023: [ If an error is encountered hsm_client_tpm_sign_data shall return NULL. ] */
TEST_FUNCTION(hsm_client_tpm_sign_data_fail)
{
    unsigned char* key;
    size_t key_len;

    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    setup_hsm_client_tpm_sign_data_mocks();

    umock_c_negative_tests_snapshot();

    size_t count = umock_c_negative_tests_call_count();
    for (size_t index = 0; index < count; index++)
    {
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        char tmp_msg[64];
        sprintf(tmp_msg, "hsm_client_tpm_sign_data failure in test %zu/%zu", index, count);

        //act
        int result = hsm_client_tpm_sign_data(NULL, TEST_BUFFER, TEST_BUFFER_SIZE, &key, &key_len);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, 0, result, tmp_msg);
    }

    //cleanup
    hsm_client_tpm_destroy(sec_handle);
    umock_c_negative_tests_deinit();
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_021: [ hsm_client_tpm_sign_data shall call into the tpm to hash the supplied data value. ] */
/* Tests_SRS_SECURE_DEVICE_TPM_07_022: [ If hashing the data was successful, hsm_client_tpm_sign_data shall create a BUFFER_HANDLE with the supplied signed data. ] */
TEST_FUNCTION(hsm_client_tpm_sign_data_succeed)
{
    unsigned char* key;
    size_t key_len;

    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    setup_hsm_client_tpm_sign_data_mocks();

    //act
    int result = hsm_client_tpm_sign_data(sec_handle, TEST_BUFFER, TEST_BUFFER_SIZE, &key, &key_len);

    //assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    my_gballoc_free(key);
    hsm_client_tpm_destroy(sec_handle);
}

#if 0
/* Tests_SRS_SECURE_DEVICE_TPM_07_025: [ If handle or data is NULL or data_len is 0, hsm_client_tpm_decrypt_data shall return NULL. ] */
TEST_FUNCTION(hsm_client_tpm_decrypt_data_handle_NULL_fail)
{
    //arrange

    //act
    BUFFER_HANDLE decrypt_value = hsm_client_tpm_decrypt_data(NULL, TEST_BUFFER, TEST_BUFFER_SIZE);

    //assert
    ASSERT_IS_NULL(decrypt_value);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_029: [ If an error is encountered secure_dev_tpm_decrypt_data shall return NULL. ] */
TEST_FUNCTION(hsm_client_tpm_decrypt_data_data_NULL_fail)
{
    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    //act
    BUFFER_HANDLE decrypt_value = hsm_client_tpm_decrypt_data(sec_handle, NULL, TEST_BUFFER_SIZE);

    //assert
    ASSERT_IS_NULL(decrypt_value);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    my_gballoc_free(decrypt_value);
    hsm_client_tpm_destroy(sec_handle);
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_025: [ If handle or data is NULL or data_len is 0, hsm_client_tpm_decrypt_data shall return NULL. ] */
TEST_FUNCTION(hsm_client_tpm_decrypt_data_data_len_0_fail)
{
    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    //act
    BUFFER_HANDLE decrypt_value = hsm_client_tpm_decrypt_data(sec_handle, TEST_BUFFER, 0);

    //assert
    ASSERT_IS_NULL(decrypt_value);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    my_gballoc_free(decrypt_value);
    hsm_client_tpm_destroy(sec_handle);
}

/* Tests_SRS_SECURE_DEVICE_TPM_07_024: [ hsm_client_tpm_decrypt_data shall call into the tpm to decrypt the supplied data value. ] */
TEST_FUNCTION(hsm_client_tpm_decrypt_data_succeed)
{
    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    TPM_SE tmp_se = 0;
    TPMA_SESSION tmp_session = { 0 };

    STRICT_EXPECTED_CALL(TSS_StartAuthSession(IGNORED_PTR_ARG, tmp_se, IGNORED_NUM_ARG, tmp_session, IGNORED_PTR_ARG))
        .IgnoreArgument_sessAttrs()
        .IgnoreArgument_sessionType();
    STRICT_EXPECTED_CALL(TSS_PolicySecret(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)); // 4
    STRICT_EXPECTED_CALL(TSS_GetTpmProperty(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(TPM2B_ID_OBJECT_Unmarshal(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TPM2B_ENCRYPTED_SECRET_Unmarshal(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TPM2B_PRIVATE_Unmarshal(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TPM2B_ENCRYPTED_SECRET_Unmarshal(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TPM2B_PUBLIC_Unmarshal(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(UINT16_Unmarshal(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TPM2_ActivateCredential(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(BUFFER_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    //act
    BUFFER_HANDLE decrypt_value = hsm_client_tpm_decrypt_data(sec_handle, TEST_BUFFER, TEST_BUFFER_SIZE);

    //assert
    ASSERT_IS_NOT_NULL(decrypt_value);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    my_gballoc_free(decrypt_value);
    hsm_client_tpm_destroy(sec_handle);
}
#endif

/* Tests_SRS_SECURE_DEVICE_TPM_07_026: [ hsm_client_tpm_interface shall return the SEC_TPM_INTERFACE structure. ] */
TEST_FUNCTION(hsm_client_tpm_interface_succeed)
{
    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_tpm_create();
    umock_c_reset_all_calls();

    //act
    const HSM_CLIENT_TPM_INTERFACE* tpm_iface = hsm_client_tpm_interface();

    //assert
    ASSERT_IS_NOT_NULL(tpm_iface);
    ASSERT_IS_NOT_NULL(tpm_iface->hsm_client_tpm_create);
    ASSERT_IS_NOT_NULL(tpm_iface->hsm_client_tpm_destroy);
    ASSERT_IS_NOT_NULL(tpm_iface->hsm_client_get_ek);
    ASSERT_IS_NOT_NULL(tpm_iface->hsm_client_get_srk);
    ASSERT_IS_NOT_NULL(tpm_iface->hsm_client_import_key);
    ASSERT_IS_NOT_NULL(tpm_iface->hsm_client_sign_data);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    hsm_client_tpm_destroy(sec_handle);
}

#endif // 0

TEST_FUNCTION(hsm_client_http_edge_sign_data_succeed)
{
    unsigned char* key;
    size_t key_len;

    //arrange
    HSM_CLIENT_HANDLE sec_handle = hsm_client_http_edge_create();
    umock_c_reset_all_calls();

    setup_hsm_client_tpm_sign_data_mocks();

    //act
    int result = hsm_client_http_edge_sign_data(sec_handle, TEST_BUFFER, TEST_BUFFER_SIZE, &key, &key_len);

    //assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    my_gballoc_free(key);
    hsm_client_tpm_destroy(sec_handle);
}



END_TEST_SUITE(hsm_client_http_edge_ut)
