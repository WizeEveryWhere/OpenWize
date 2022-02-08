#include "unity_fixture.h"

TEST_GROUP_RUNNER(Samples_ImgStorage)
{
    RUN_TEST_CASE(Samples_ImgStorage, test_Imgstore_Setup);
    RUN_TEST_CASE(Samples_ImgStorage, test_Imgstore_Init);
    RUN_TEST_CASE(Samples_ImgStorage, test_Imgstore_IsComplete);
    RUN_TEST_CASE(Samples_ImgStorage, test_Imgstore_Verify);
    RUN_TEST_CASE(Samples_ImgStorage, test_Imgstore_StoreBlock);
}

TEST_GROUP_RUNNER(Samples_ImgStorageInt)
{
    RUN_TEST_CASE(Samples_ImgStorageInt, test_ImgstoreInt_ClrBitmap);
    RUN_TEST_CASE(Samples_ImgStorageInt, test_ImgstoreInt_SetBitmap);
    RUN_TEST_CASE(Samples_ImgStorageInt, test_ImgstoreInt_GetLineBitmap);
    RUN_TEST_CASE(Samples_ImgStorageInt, test_ImgstoreInt_GetBitmap);
    RUN_TEST_CASE(Samples_ImgStorageInt, test_ImgstoreInt_ClrPending);
}
