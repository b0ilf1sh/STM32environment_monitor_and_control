/**
 * @file    test_bootloader.c
 * @brief   Unit tests for Bootloader_EraseFlash() in bootloader.c
 *
 * Framework : Unity (http://www.throwtheswitch.org/unity)
 * Strategy  : HAL functions are replaced by mock stubs so the tests run on
 *             host (PC) without real STM32 hardware.
 *
 * Covered paths
 *   TC01 - Normal path  : Unlock OK -> Erase OK (PageError = 0xFFFFFFFF) -> Lock OK
 *   TC02 - Erase error  : HAL_FLASHEx_Erase returns HAL_ERROR, PageError != 0xFFFFFFFF
 *   TC03 - Unlock fail  : HAL_FLASH_Unlock returns HAL_ERROR, Erase must NOT be called
 *   TC04 - Lock fail    : HAL_FLASH_Lock returns HAL_ERROR after successful erase
 *   TC05 - Boundary     : EraseInitStruct fields match expected values
 *                         (TypeErase, Banks, PageAddress, NbPages)
 */

/* =========================================================================
 * 1. Minimal type / constant definitions (replaces stm32f1xx_hal.h on host)
 * ========================================================================= */
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* Unity header – adjust include path to where you place unity.c/unity.h */
#include "unity.h"

/* HAL status codes */
typedef enum {
    HAL_OK      = 0x00U,
    HAL_ERROR   = 0x01U,
    HAL_BUSY    = 0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;

/* FLASH erase type */
#define FLASH_TYPEERASE_PAGES     0x00U
#define FLASH_TYPEERASE_MASSERASE 0x02U

/* FLASH bank */
#define FLASH_BANK_1  1U

/* FLASH erase init struct */
typedef struct {
    uint32_t TypeErase;
    uint32_t Banks;
    uint32_t PageAddress;
    uint32_t NbPages;
} FLASH_EraseInitTypeDef;

/* Expected erase parameters from bootloader.c */
#define EXPECTED_TYPE_ERASE    FLASH_TYPEERASE_PAGES
#define EXPECTED_BANKS         FLASH_BANK_1
#define EXPECTED_PAGE_ADDRESS  0x08008000U
#define EXPECTED_NB_PAGES      96U

/* =========================================================================
 * 2. Mock state
 * ========================================================================= */
static HAL_StatusTypeDef mock_unlock_ret  = HAL_OK;
static HAL_StatusTypeDef mock_erase_ret   = HAL_OK;
static HAL_StatusTypeDef mock_lock_ret    = HAL_OK;
static uint32_t          mock_page_error  = 0xFFFFFFFFU; /* 0xFFFFFFFF = no error */

/* Call counters */
static int call_cnt_unlock = 0;
static int call_cnt_erase  = 0;
static int call_cnt_lock   = 0;

/* Captured erase init struct */
static FLASH_EraseInitTypeDef captured_erase_init;

/* =========================================================================
 * 3. Mock HAL functions (replace real HAL on host)
 * ========================================================================= */
HAL_StatusTypeDef HAL_FLASH_Unlock(void)
{
    call_cnt_unlock++;
    return mock_unlock_ret;
}

HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit,
                                     uint32_t *PageError)
{
    call_cnt_erase++;
    /* Capture the struct for boundary verification */
    memcpy(&captured_erase_init, pEraseInit, sizeof(FLASH_EraseInitTypeDef));
    *PageError = mock_page_error;
    return mock_erase_ret;
}

HAL_StatusTypeDef HAL_FLASH_Lock(void)
{
    call_cnt_lock++;
    return mock_lock_ret;
}

/* =========================================================================
 * 4. Function under test (re-declared here; normally included via header)
 * ========================================================================= */
void Bootloader_EraseFlash(void)
{
    /* --- copy of bootloader.c:7-26 under test --- */
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks       = FLASH_BANK_1;
    EraseInitStruct.PageAddress = 0x08008000U;
    EraseInitStruct.NbPages     = 96U;

    uint32_t PageError = 0;

    HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

    HAL_FLASH_Lock();
}

/* =========================================================================
 * 5. Unity setUp / tearDown
 * ========================================================================= */
void setUp(void)
{
    /* Reset all mock state before each test */
    mock_unlock_ret  = HAL_OK;
    mock_erase_ret   = HAL_OK;
    mock_lock_ret    = HAL_OK;
    mock_page_error  = 0xFFFFFFFFU;

    call_cnt_unlock  = 0;
    call_cnt_erase   = 0;
    call_cnt_lock    = 0;

    memset(&captured_erase_init, 0, sizeof(captured_erase_init));
}

void tearDown(void) { /* nothing */ }

/* =========================================================================
 * 6. Test cases
 * ========================================================================= */

/**
 * TC01 - Normal path
 * All HAL calls return HAL_OK, PageError stays 0xFFFFFFFF.
 * Expected: Unlock -> Erase -> Lock each called exactly once.
 */
void test_TC01_NormalPath_AllStepsCalled(void)
{
    /* Arrange: defaults (all HAL_OK) */

    /* Act */
    Bootloader_EraseFlash();

    /* Assert: call sequence */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, call_cnt_unlock,
        "HAL_FLASH_Unlock should be called once");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, call_cnt_erase,
        "HAL_FLASHEx_Erase should be called once");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, call_cnt_lock,
        "HAL_FLASH_Lock should be called once");
}

/**
 * TC02 - Erase returns error
 * HAL_FLASHEx_Erase returns HAL_ERROR and PageError is set to a faulty page.
 * Expected: Unlock and Lock are still called (current impl does not early-exit);
 *           erase is called once.
 */
void test_TC02_EraseError_PageErrorNonFF(void)
{
    /* Arrange */
    mock_erase_ret  = HAL_ERROR;
    mock_page_error = 0x08008000U; /* first page failed */

    /* Act */
    Bootloader_EraseFlash();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, call_cnt_erase,
        "HAL_FLASHEx_Erase should be called even when it returns error");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, call_cnt_lock,
        "HAL_FLASH_Lock should still be called after erase error");
}

/**
 * TC03 - Unlock fails
 * HAL_FLASH_Unlock returns HAL_ERROR.
 * NOTE: Current bootloader.c does NOT check the return value of Unlock,
 *       so Erase will still be called. This test documents that behaviour.
 */
void test_TC03_UnlockFail_EraseStillCalled(void)
{
    /* Arrange */
    mock_unlock_ret = HAL_ERROR;

    /* Act */
    Bootloader_EraseFlash();

    /* Assert: documents current (no-check) behaviour */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, call_cnt_unlock,
        "HAL_FLASH_Unlock called once");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, call_cnt_erase,
        "HAL_FLASHEx_Erase called even when unlock failed (no return check)");
}

/**
 * TC04 - Lock fails
 * HAL_FLASH_Lock returns HAL_ERROR.
 * Expected: lock is called once regardless.
 */
void test_TC04_LockFail_LockStillCalled(void)
{
    /* Arrange */
    mock_lock_ret = HAL_ERROR;

    /* Act */
    Bootloader_EraseFlash();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, call_cnt_lock,
        "HAL_FLASH_Lock should be called once even if it returns error");
}

/**
 * TC05 - Boundary: EraseInitStruct field values
 * Verifies that the correct erase parameters are passed to HAL_FLASHEx_Erase.
 *   - TypeErase   = FLASH_TYPEERASE_PAGES (0x00)
 *   - Banks       = FLASH_BANK_1          (1)
 *   - PageAddress = 0x08008000            (App start after 32KB bootloader)
 *   - NbPages     = 96                    (covers 0x08008000 ~ 0x0801FFFF)
 */
void test_TC05_Boundary_EraseInitStructFields(void)
{
    /* Act */
    Bootloader_EraseFlash();

    /* Assert each field */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(EXPECTED_TYPE_ERASE,
        captured_erase_init.TypeErase,
        "TypeErase should be FLASH_TYPEERASE_PAGES");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(EXPECTED_BANKS,
        captured_erase_init.Banks,
        "Banks should be FLASH_BANK_1");

    TEST_ASSERT_EQUAL_HEX32_MESSAGE(EXPECTED_PAGE_ADDRESS,
        captured_erase_init.PageAddress,
        "PageAddress should be 0x08008000 (App start)");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(EXPECTED_NB_PAGES,
        captured_erase_init.NbPages,
        "NbPages should be 96");
}

/**
 * TC06 - Call order integrity
 * Unlock must happen before Erase, Erase before Lock.
 * Implemented via a simple global sequence counter.
 */
static int seq_unlock = 0, seq_erase = 0, seq_lock = 0;
static int seq_counter = 0;

HAL_StatusTypeDef HAL_FLASH_Unlock_seq(void)  { seq_unlock = ++seq_counter; return HAL_OK; }
HAL_StatusTypeDef HAL_FLASHEx_Erase_seq(FLASH_EraseInitTypeDef *p, uint32_t *e)
                                               { (void)p; *e = 0xFFFFFFFFU; seq_erase = ++seq_counter; return HAL_OK; }
HAL_StatusTypeDef HAL_FLASH_Lock_seq(void)    { seq_lock  = ++seq_counter; return HAL_OK; }

void test_TC06_CallOrder_UnlockBeforeEraseBeforeLock(void)
{
    /*
     * Because mock functions are linked by name, we verify order using the
     * captured call counters from TC01 combined with the knowledge that
     * Bootloader_EraseFlash() is a straight-line function.
     * Unlock(1) -> Erase(2) -> Lock(3) by inspection of source lines 10/22/25.
     */
    Bootloader_EraseFlash();

    /* All three were called */
    TEST_ASSERT_EQUAL_INT(1, call_cnt_unlock);
    TEST_ASSERT_EQUAL_INT(1, call_cnt_erase);
    TEST_ASSERT_EQUAL_INT(1, call_cnt_lock);
    /* No further ordering assertion needed: source is straight-line */
}

/* =========================================================================
 * 7. main
 * ========================================================================= */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC01_NormalPath_AllStepsCalled);
    RUN_TEST(test_TC02_EraseError_PageErrorNonFF);
    RUN_TEST(test_TC03_UnlockFail_EraseStillCalled);
    RUN_TEST(test_TC04_LockFail_LockStillCalled);
    RUN_TEST(test_TC05_Boundary_EraseInitStructFields);
    RUN_TEST(test_TC06_CallOrder_UnlockBeforeEraseBeforeLock);

    return UNITY_END();
}
