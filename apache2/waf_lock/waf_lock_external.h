#ifndef _WAF_LOCK_EXTERNAL_HEADER
#define _WAF_LOCK_EXTERNAL_HEADER

#include "stdbool.h"
#ifdef _WIN32
#ifdef inline
#undef inline
#endif
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 **========================================================
 ** Lock Status Definition
 **========================================================
 */
/**
 ** First Digit: equals 1 if it is an error code.
 ** Second Digit: the platform where the error happens.
 **              0 means general, 1 means LINUX, 2 means WINDOWS.
 ** 3th & 4th Digit: the detailed error number.
 */

#define WAF_LOCK_SUCCESS                                        0000
#define WAF_SUCCESS_LOCK_CREATE                                 0001
#define WAF_SUCCESS_LOCK_OPEN                                   0002

#define WAF_LOCK_ERROR                                          1000
#define WAF_ERROR_LOCK_LINUX_SEM_CREATE_FAIL                    1100
#define WAF_ERROR_LOCK_LINUX_SEM_OPEN_FAIL                      1101
#define WAF_ERROR_LOCK_LINUX_SEM_INIT_FAIL                      1102
#define WAF_ERROR_LOCK_LINUX_SEM_MODIFY_FAIL                    1103
#define WAF_ERROR_LOCK_LINUX_SEM_DESTROY_FAIL                   1104
#define	WAF_ERROR_LOCK_LINUX_SEM_SET_PERMISSION_FAIL            1105
#define	WAF_ERROR_LOCK_LINUX_SEM_GET_USER_FAIL                  1106

#define WAF_ERROR_LOCK_WIN_NAME_INVALID_STRING                  1200
#define WAF_ERROR_LOCK_WIN_MUTEX_CREATE_FAIL                    1201
#define WAF_ERROR_LOCK_WIN_ONLY_ONE_LOCK_EXISTS                 1202
#define WAF_ERROR_LOCK_WIN_GET_MUTEX_FAIL                       1203
#define WAF_ERROR_LOCK_WIN_RELEASE_MUTEX_FAIL                   1204
#define WAF_ERROR_LOCK_WIN_CLOSE_MUTEX_FAIL                     1205
#define WAF_ERROR_LOCK_WIN_DESTROY_NOT_SUPPORT                  1206

#define WAF_LOCK_ERROR_NAME_NULL                                1001
#define WAF_LOCK_ERROR_NAME_INVALID_STRING                      1002
#define WAF_ERROR_LOCK_OP_NEGATIVE_VAL                          1003
#define WAF_LOCK_ERROR_HANDLE_NULL                              1004
#define WAF_LOCK_ERROR_UNEXPECTED                               1005


/**
 **========================================================
 ** Waf Lock Handler Definition
 **========================================================
 */

/**
 ** The structure stores the Read/Write locks information.
 ** In Linux, a set of semaphores is used to implement the Read/Write locks.
 ** In Windows, The Windows Mutex is used to implement the Read/Write locks.
 ** @param sem_id: The identifier of semaphore set in Linux.
 ** @param read_lock_handle: The handler of the read locks in Windows.
 ** @param write_lock_handle: The handler of the write lock in Windows.
 */
struct waf_lock {
#ifndef _WIN32
    int sem_id;
#else
    HANDLE read_lock_handle;
    HANDLE write_lock_handle;
#endif
};


/**
 **========================================================
 ** Waf Lock API
 **========================================================
 */

/**
 ** Create a lock.
 ** @param lock: lock handler.
 ** return: WAF_LOCK_SUCCESS if successfully created or WAF_LOCK_ERROR if failed.
 */
int Waf_createLock(struct waf_lock *lock, const char* lock_name, int lock_name_length, const char* owner);

/**
 ** Destroy a lock.
 ** @param lock: lock handler.
 ** return: WAF_LOCK_SUCCESS if successfully created or WAF_LOCK_ERROR if failed.
 */
int Waf_destroyLock(struct waf_lock *lock);

/**
 ** Close a lock.
 ** @param lock: lock handler.
 ** return: WAF_LOCK_SUCCESS if successfully created or WAF_LOCK_ERROR if failed.
 */
int Waf_cLoseLock(struct waf_lock *lock);

/**
 ** Get a shared lock for read only.
 ** @param lock: lock handler.
 ** return: WAF_LOCK_SUCCESS if successfully created or WAF_LOCK_ERROR if failed.
 */

int Waf_getSharedLock(struct waf_lock *lock);

/**
 ** Get a exclusive lock for read and write.
 ** @param lock: lock handler.
 ** return: WAF_LOCK_SUCCESS if successfully created or WAF_LOCK_ERROR if failed.
 */
int Waf_getExclusiveLock(struct waf_lock *lock);

/**
 ** Free a shared lock that you have got before.
 ** @param lock: lock handler.
 ** return: WAF_LOCK_SUCCESS if successfully created or WAF_LOCK_ERROR if failed.
 */
int Waf_freeSharedLock(struct waf_lock *lock);

/**
 ** Free a exclusive lock that you have got before.
 ** @param lock: lock handler.
 ** return: WAF_LOCK_SUCCESS if successfully created or WAF_LOCK_ERROR if failed.
 */
int Waf_freeExclusiveLock(struct waf_lock *lock);

/**
 **========================================================
 ** Lock Debug API
 **========================================================
 */
/**
 ** Check whether a return_code is an error.
 ** @param return_code: the code returned by a Lock function.
 ** return: True if there is an error;
            False if not.
 */
bool Waf_lock_isError(int return_code);

#ifdef __cplusplus
}
#endif

#endif

