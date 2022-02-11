#include "unity_fixture.h"

TEST_GROUP_RUNNER(Samples_Crypto)
{
    RUN_TEST_CASE(Samples_Crypto, test_Crypto_Encrypt16_Success);
    RUN_TEST_CASE(Samples_Crypto, test_Crypto_Encrypt32_Success);
    RUN_TEST_CASE(Samples_Crypto, test_Crypto_Encrypt36_Success);
    RUN_TEST_CASE(Samples_Crypto, test_Crypto_Encrypt_Mismatch);

    RUN_TEST_CASE(Samples_Crypto, test_Crypto_Encrypt_Fail);
    RUN_TEST_CASE(Samples_Crypto, test_Crypto_Encrypt_BadKey);
    RUN_TEST_CASE(Samples_Crypto, test_Crypto_Encrypt_Key0);
    RUN_TEST_CASE(Samples_Crypto, test_Crypto_Encrypt_NullPointer);
    RUN_TEST_CASE(Samples_Crypto, test_Crypto_Decrypt16_Success);
    RUN_TEST_CASE(Samples_Crypto, test_Crypto_AES128_CMAC_Kenc_Success);
    RUN_TEST_CASE(Samples_Crypto, test_Crypto_AES128_CMAC_Kmac_Success);
    RUN_TEST_CASE(Samples_Crypto, test_Crypto_AES128_CMAC_Mismatch);
    RUN_TEST_CASE(Samples_Crypto, test_Crypto_AES128_CMAC_Fail);
    RUN_TEST_CASE(Samples_Crypto, test_Crypto_AES128_CMAC_BadKey);
    RUN_TEST_CASE(Samples_Crypto, test_Crypto_AES128_CMAC_NullPointer);

}
