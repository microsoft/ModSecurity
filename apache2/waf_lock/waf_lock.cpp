#include"waf_lock_external.h"
#include"waf_lock_internal.h"


/**
**========================================================
** Waf Lock Internal Function
**========================================================
*/

/**
** Create and initialize a waf_lock, if the lock with given key exists, just link to the lock.
** The new_lock should have been created before calling this function.
** @param new_lock:        The pointer to save the information of the lock.
** @param lock_name:       The name of lock.
** @param lock_name_length:The length of the lock_name.
** return: WAF_SUCCESS_LOCK_CREATE if successfully created a new lock,
**         WAF_SUCCESS_LOCK_OPEN if successfully link to an existed lock,
**         WAF_LOCK_ERROR if failed.
*/
int lock_create(struct waf_lock *new_lock, const char* lock_name, int lock_name_length, const char* owner) {
#ifndef _WIN32
    union semun sem_union;
    int lock_id = -1;
    uid_t uid;
    gid_t gid;
#else
    int read_lock_name_len = 0;
    int write_lock_name_len = 0;
    char* read_lock_name = NULL;
    char* write_lock_name = NULL;
    bool lock_exists = false;
#endif
    if (new_lock == NULL)
        return WAF_LOCK_ERROR_HANDLE_NULL;
#ifndef _WIN32
    lock_id = Waf_lock_hash(lock_name, lock_name_length, DEFAULT_WAF_LOCK_ID_RANGE);
    new_lock->sem_id = semget(lock_id, SEM_NUMBERS, IPC_CREAT | IPC_EXCL);
    if (new_lock->sem_id != -1) {
        // A new semaphore set is created, need to initialize the semaphore set
	// Set permssion
	struct semid_ds buf;
	if ((GetUserId(owner, &uid) == WAF_LOCK_ERROR) || (GetGroupId(owner, &gid) == WAF_LOCK_ERROR)) {
            lock_destroy(new_lock);
            return WAF_ERROR_LOCK_LINUX_SEM_GET_USER_FAIL;
	}

	buf.sem_perm.uid = uid; 
	buf.sem_perm.gid = gid; 
	buf.sem_perm.mode = 0600;
	sem_union.buf = &buf;
        if (semctl(new_lock->sem_id, 0, IPC_SET, sem_union) == -1) {
            lock_destroy(new_lock);
            return WAF_ERROR_LOCK_LINUX_SEM_SET_PERMISSION_FAIL;
        }
	
        sem_union.val = SEM_READ_INITVAL;
        if (semctl(new_lock->sem_id, SEM_ID_READ, SETVAL, sem_union) == -1) {
            lock_destroy(new_lock);
            return WAF_ERROR_LOCK_LINUX_SEM_CREATE_FAIL;
        }

        sem_union.val = SEM_WRITE_INITVAL;
        if (semctl(new_lock->sem_id, SEM_ID_WRITE, SETVAL, sem_union) == -1) {
            // If failed, destroy the lock
            lock_destroy(new_lock);
            return WAF_ERROR_LOCK_LINUX_SEM_INIT_FAIL;
        }
        return WAF_SUCCESS_LOCK_CREATE;
    }
    else {
        new_lock->sem_id = semget(lock_id, SEM_NUMBERS, IPC_CREAT);
        if (new_lock->sem_id == -1) {
            return WAF_ERROR_LOCK_LINUX_SEM_OPEN_FAIL;
        }
        return WAF_SUCCESS_LOCK_OPEN;
    }
#else
    read_lock_name_len = (strlen(READ_LOCK_SUFFIX) + strlen(lock_name) + 1) * sizeof(char);
    write_lock_name_len = (strlen(WRITE_LOCK_SUFFIX) + strlen(lock_name) + 1) * sizeof(char);

    if (Waf_lock_isstring(lock_name, lock_name_length) != WAF_LOCK_SUCCESS)
        return WAF_ERROR_LOCK_WIN_NAME_INVALID_STRING;

    read_lock_name = (char *)malloc(read_lock_name_len * sizeof(char));
    sprintf_s(read_lock_name, read_lock_name_len, "%s%s", lock_name, READ_LOCK_SUFFIX);
    new_lock->read_lock_handle = CreateMutex(
        NULL,               // Default security settings.
        FALSE,              // Do not take the lock after created.
        read_lock_name);    // The name of read lock.
    free(read_lock_name);

    if (new_lock->read_lock_handle == NULL) {
        new_lock->read_lock_handle = INVALID_HANDLE_VALUE;
        return WAF_ERROR_LOCK_WIN_MUTEX_CREATE_FAIL;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS)
        lock_exists = true;

    write_lock_name = (char *)malloc(write_lock_name_len * sizeof(char));
    sprintf_s(write_lock_name, write_lock_name_len, "%s%s", lock_name, WRITE_LOCK_SUFFIX);
    new_lock->write_lock_handle = CreateMutex(
        NULL,               // Default security settings.
        FALSE,              // Do not take the lock after created.
        write_lock_name);   // The name of write lock.
    free(write_lock_name);

    if (new_lock->write_lock_handle == NULL) {
        CloseHandle(new_lock->read_lock_handle);
        new_lock->read_lock_handle = INVALID_HANDLE_VALUE;
        new_lock->write_lock_handle = INVALID_HANDLE_VALUE;
        return WAF_ERROR_LOCK_WIN_MUTEX_CREATE_FAIL;
    }

    if ((GetLastError() == ERROR_ALREADY_EXISTS) != lock_exists) {
        // One lock exists, another not.
        CloseHandle(new_lock->read_lock_handle);
        new_lock->read_lock_handle = INVALID_HANDLE_VALUE;
        CloseHandle(new_lock->write_lock_handle);
        new_lock->write_lock_handle = INVALID_HANDLE_VALUE;
        return WAF_ERROR_LOCK_WIN_ONLY_ONE_LOCK_EXISTS;
    }

    if (lock_exists == true)
        return WAF_SUCCESS_LOCK_OPEN;
    else
        return WAF_SUCCESS_LOCK_CREATE;
#endif
    // Should never get here
    return WAF_SUCCESS_LOCK_CREATE;
}

/**
** Destroy the lock
** @param waf_lock: The waf_lock sturcture
** return: WAF_LOCK_SUCCESS if successfully destroy the lock,
**         WAF_LOCK_ERROR if failed.
*/
int lock_destroy(struct waf_lock *waf_lock) {
    int rc = WAF_LOCK_SUCCESS;
    if (waf_lock == NULL)
        return WAF_LOCK_ERROR_HANDLE_NULL;
    rc = lock_close(waf_lock);
    if (Waf_lock_isError(rc))
        return rc;
#ifndef _WIN32
    if (waf_lock->sem_id == -1)
        return WAF_LOCK_SUCCESS;
    rc = semctl(waf_lock->sem_id, 0, IPC_RMID);
    if (rc == -1)
        return WAF_ERROR_LOCK_LINUX_SEM_DESTROY_FAIL;
    else
        waf_lock->sem_id = -1;
    return WAF_LOCK_SUCCESS;
#else
    /* Locks destroy doesn't support on Windows */
    return WAF_ERROR_LOCK_WIN_DESTROY_NOT_SUPPORT;
#endif
}

/**
** Close the lock
** @param waf_lock: The waf_lock sturcture
** return: WAF_LOCK_SUCCESS if successfully close the lock,
**         WAF_LOCK_ERROR if failed.
*/
int lock_close(struct waf_lock *waf_lock) {
    if (waf_lock == NULL)
        return WAF_LOCK_ERROR_HANDLE_NULL;
    /* Linux doesn't do anything */
#ifdef _WIN32
    BOOL rc_read = 1;
    BOOL rc_write = 1;
    if (waf_lock->read_lock_handle != INVALID_HANDLE_VALUE)
    {
        rc_read = CloseHandle(waf_lock->read_lock_handle);
        if (rc_read != 0)
            waf_lock->read_lock_handle = INVALID_HANDLE_VALUE;
    }

    if (waf_lock->write_lock_handle != INVALID_HANDLE_VALUE)
    {
        rc_write = CloseHandle(waf_lock->write_lock_handle);
        if (rc_write != 0)
            waf_lock->write_lock_handle = INVALID_HANDLE_VALUE;
    }

    if (rc_read == 0 || rc_write == 0)
        return WAF_ERROR_LOCK_WIN_CLOSE_MUTEX_FAIL;
#endif
    return WAF_LOCK_SUCCESS;
}

/**
** Decrease a lock's value by a given number.
** @param waf_lock: The waf_lock sturcture
** @param index:   The index of atom lock (read or write) .
** @param val:     The value you want to decrease from the lock.
** return: WAF_LOCK_SUCCESS if success;
**         or WAF_LOCK_ERROR if failed
*/
int lock_P(const struct waf_lock *waf_lock, int index, int val) {
#ifndef _WIN32
    struct sembuf sem_op;
    if (val < 0)
        return WAF_ERROR_LOCK_OP_NEGATIVE_VAL;

    sem_op.sem_num = index;
    sem_op.sem_op = -val;
    sem_op.sem_flg = SEM_UNDO;
    if (semop(waf_lock->sem_id, &sem_op, 1) == -1)
        return WAF_ERROR_LOCK_LINUX_SEM_MODIFY_FAIL;
    return WAF_LOCK_SUCCESS;
#else
    int rc;
    HANDLE lock_handle;
    if (val < 0)
        return WAF_ERROR_LOCK_OP_NEGATIVE_VAL;

    if (index == SEM_ID_READ)
        lock_handle = waf_lock->read_lock_handle;
    else
        lock_handle = waf_lock->write_lock_handle;

    if (lock_handle == INVALID_HANDLE_VALUE)
        return WAF_ERROR_LOCK_WIN_GET_MUTEX_FAIL;

    rc = WaitForSingleObject(lock_handle, INFINITE);
    if (rc != WAIT_OBJECT_0)
        return WAF_ERROR_LOCK_WIN_GET_MUTEX_FAIL;
    return WAF_LOCK_SUCCESS;
#endif
}

/**
** Increase a lock's value by a given number.
** @param waf_lock: The waf_lock sturcture
** @param index:   The index of atom lock (read or write) .
** @param val:     The value you want to add to the lock.
** return: WAF_LOCK_SUCCESS if success;
**         or WAF_LOCK_ERROR if failed
*/
int lock_V(const struct waf_lock *waf_lock, int index, int val) {
#ifndef _WIN32
    struct sembuf sem_op;
    if (val < 0)
        return WAF_ERROR_LOCK_OP_NEGATIVE_VAL;

    sem_op.sem_num = index;
    sem_op.sem_op = val;
    sem_op.sem_flg = SEM_UNDO;
    if (semop(waf_lock->sem_id, &sem_op, 1) == -1)
        return WAF_ERROR_LOCK_LINUX_SEM_MODIFY_FAIL;
    return WAF_LOCK_SUCCESS;
#else
    int rc;
    HANDLE lock_handle;
    if (val < 0)
        return WAF_ERROR_LOCK_OP_NEGATIVE_VAL;

    if (index == SEM_ID_READ)
        lock_handle = waf_lock->read_lock_handle;
    else
        lock_handle = waf_lock->write_lock_handle;

    if (lock_handle == INVALID_HANDLE_VALUE)
        return WAF_ERROR_LOCK_WIN_RELEASE_MUTEX_FAIL;

    rc = ReleaseMutex(lock_handle);
    if (rc == 0)
        return WAF_ERROR_LOCK_WIN_RELEASE_MUTEX_FAIL;
    return WAF_LOCK_SUCCESS;
#endif
}

/**
** Check the string format.
** @param str: the string you want to check.
** @param str_len: given string length (not include '\0').
** return: WAF_LOCK_SUCCESS if the string length equals str_len;
**             WAF_LOCK_ERROR if not
*/
int Waf_lock_isstring(const char* str, int str_len) {
    int i;
    for (i = 0;i < str_len;i++)
        if (str[i] == '\0')
            return WAF_LOCK_ERROR;
    if (str[i] != '\0')
        return WAF_LOCK_ERROR;
    return WAF_LOCK_SUCCESS;
}

/**
** Check whether a return_code is an error.
** @param return_code: the code returned by a Waf Lock function.
** return: True if there is an error;
False if not.
*/
bool Waf_lock_isError(int return_code) {
    if (return_code < WAF_LOCK_ERROR)
        return false;
    else
        return true;
}

/**
** Use WAF lock's hash function to hash a string into an integer.
** @param key: the key string.
** @param key_len: the length of the key string.
** @param output_val_range: the range of output value.
** return: hash result if success
**      or WAF_LOCK_ERROR if failed.
*/
unsigned int Waf_lock_hash(const char* key, int key_len, unsigned int output_val_range) {
    unsigned char buf[DEFAULT_HASH_VALUE_STR_LENGTH] = { 0 };
    unsigned int hash = 0;

    MurmurHash3_x86_32(key, key_len, DEFAULT_HASH_MAGIC_SEED, buf);
    hash = ((buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]) % (output_val_range);
    return hash;
}

/**
** Initialize the handle of Waf Lock.
** @param waf_lock: the handle of Waf Lock.
** return:  if WAF_LOCK_SUCCESS if success
**          or WAF_LOCK_ERROR if the handle is NULL.
*/
int Waf_lock_init(struct waf_lock* waf_lock) {
    if (waf_lock == NULL)
        return WAF_LOCK_ERROR_HANDLE_NULL;
#ifndef _WIN32
    waf_lock->sem_id = -1;
#else
    waf_lock->read_lock_handle = INVALID_HANDLE_VALUE;
    waf_lock->write_lock_handle = INVALID_HANDLE_VALUE;
#endif
    return WAF_LOCK_SUCCESS;
}

/**
**========================================================
** Waf Lock External Function
**========================================================
*/

/**
** Get a shared lock for read only.
** @param waf_lock: lock handler you want to lock.
** return: WAF_LOCK_SUCCESS if successfully created or WAF_LOCK_ERROR if failed.
*/
int Waf_getSharedLock(struct waf_lock *waf_lock) {
    int rc;
    if (waf_lock == NULL)
        return WAF_LOCK_ERROR_HANDLE_NULL;

    rc = lock_P(waf_lock, SEM_ID_WRITE, 1);
    if (rc != WAF_LOCK_SUCCESS)
        return rc;

    rc = lock_P(waf_lock, SEM_ID_READ, 1);
    if (rc != WAF_LOCK_SUCCESS) {
        lock_V(waf_lock, SEM_ID_WRITE, 1);
        return rc;
    }

    rc = lock_V(waf_lock, SEM_ID_WRITE, 1);
    if (rc != WAF_LOCK_SUCCESS) {
        lock_V(waf_lock, SEM_ID_READ, 1);
        return rc;
    }

    return WAF_LOCK_SUCCESS;
}

/**
** Get a exclusive lock for read and write.
** @param waf_lock: the lock handler you want to return the lock.
** return: WAF_LOCK_SUCCESS if successfully created or WAF_LOCK_ERROR if failed.
*/
int Waf_getExclusiveLock(struct waf_lock *waf_lock) {
    int rc;
    if (waf_lock == NULL)
        return WAF_LOCK_ERROR_HANDLE_NULL;

    rc = lock_P(waf_lock, SEM_ID_WRITE, 1);
    if (rc != WAF_LOCK_SUCCESS)
        return rc;

    rc = lock_P(waf_lock, SEM_ID_READ, SEM_READ_INITVAL);
    if (rc != WAF_LOCK_SUCCESS) {
        lock_V(waf_lock, SEM_ID_WRITE, 1);
        return rc;
    }

    return WAF_LOCK_SUCCESS;
}

/**
** Free a shared lock that you have got before.
** @param waf_lock: the lock handler you want to return the lock.
** return: WAF_LOCK_SUCCESS if successfully created or WAF_LOCK_ERROR if failed.
*/
int Waf_freeSharedLock(struct waf_lock *waf_lock) {
    int rc;
    if (waf_lock == NULL)
        return WAF_LOCK_ERROR_HANDLE_NULL;

    rc = lock_V(waf_lock, SEM_ID_READ, 1);
    if (rc != WAF_LOCK_SUCCESS)
        return rc;

    return WAF_LOCK_SUCCESS;
}

/**
** Free a exclusive lock that you have got before.
** @param waf_lock: the lock handler you want to return the lock.
** return: WAF_LOCK_SUCCESS if successfully created or WAF_LOCK_ERROR if failed.
*/
int Waf_freeExclusiveLock(struct waf_lock *waf_lock) {
    int rc;
    if (waf_lock == NULL)
        return WAF_LOCK_ERROR_HANDLE_NULL;

    rc = lock_V(waf_lock, SEM_ID_READ, SEM_READ_INITVAL);
    if (rc != WAF_LOCK_SUCCESS)
        return rc;
    rc = lock_V(waf_lock, SEM_ID_WRITE, 1);
    if (rc != WAF_LOCK_SUCCESS)
        return rc;

    return WAF_LOCK_SUCCESS;
}
/**
** Open a Lock with given name, and intialize the lock handler for further operation.
** If the lock doesn't exist, a new lock will be created.
** @param lock: a created sturcture to save the lock information.
** @param lock_name: the unique identifier of a lock.
** @param lock_name_length: the length of the unique_name.
** return: WAF_LOCK_SUCCESS if successfully created or WAF_LOCK_ERROR if failed.
*/
int Waf_createLock(struct waf_lock* waf_lock, const char* waf_lock_name, int waf_lock_name_length, const char* owner) {
    int rc = WAF_LOCK_SUCCESS;

    rc = Waf_lock_init(waf_lock);
    if (rc != WAF_LOCK_SUCCESS) {
        return rc;
    }

    /* Check the format of db_name */
    if (waf_lock_name == NULL) {
        return WAF_LOCK_ERROR_NAME_NULL;
    }
    else if (waf_lock_name_length <= 0) {
        return WAF_LOCK_ERROR_NAME_INVALID_STRING;
    }
    else if (Waf_lock_isstring(waf_lock_name, waf_lock_name_length) == WAF_LOCK_ERROR) {
        return WAF_LOCK_ERROR_NAME_INVALID_STRING;
    }

    /* Create or open the lock */
    rc = lock_create(waf_lock, waf_lock_name, waf_lock_name_length, owner);
    if (Waf_lock_isError(rc))
    {
        return rc;
    }
    else if (rc != WAF_SUCCESS_LOCK_CREATE && rc != WAF_SUCCESS_LOCK_OPEN) {
        return WAF_LOCK_ERROR_UNEXPECTED;
    }

    return rc;
}

/**
** Close and destroy a lock.
** @param waf_lock: the waf_lock you want to destroy.
** return: WAF_LOCK_SUCCESS if successfully destroyed or WAF_LOCK_ERROR if failed.
*/
int Waf_destroyLock(struct waf_lock *waf_lock) {
    int rc_lock = WAF_LOCK_SUCCESS;

    if (waf_lock == NULL)
        return WAF_LOCK_ERROR_HANDLE_NULL;

    rc_lock = lock_destroy(waf_lock);

    if (Waf_lock_isError(rc_lock))
        return rc_lock;
    else
        return WAF_LOCK_SUCCESS;
}

/**
** Close a Lock, but does not destroy it.
** @param lock: the lock you want to close.
** return: WAF_LOCK_SUCCESS if successfully closed or WAF_LOCK_ERROR if failed.
*/
int Waf_closeLock(struct waf_lock *waf_lock) {
    int rc_lock = WAF_LOCK_SUCCESS;

    if (waf_lock == NULL)
        return WAF_LOCK_ERROR_HANDLE_NULL;

    rc_lock = lock_close(waf_lock);
    if (Waf_lock_isError(rc_lock))
        return rc_lock;
    else
        return WAF_LOCK_SUCCESS;
}

int GetGroupId(const char *name, gid_t *id)
{
    struct group *grp = getgrnam(name); /* don't free, see getgrnam() for details */
    if(grp == NULL) {
        return WAF_LOCK_ERROR;
    } 

    *id = grp->gr_gid;
    return WAF_LOCK_SUCCESS;
}

int GetUserId(const char *name, uid_t *id)
{
    struct passwd *pwd = getpwnam(name); /* don't free, see getpwnam() for details */
    if(pwd == NULL) {
        return WAF_LOCK_ERROR;
    } 

    *id = pwd->pw_uid;
    return WAF_LOCK_SUCCESS;
}
