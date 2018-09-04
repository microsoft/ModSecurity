// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ri_api.h"

#define OVECCOUNT 30

int test(
    const char *pattern_str, 
    const char *subject_str, 
    int comp_opt,   // Options for compilation
    int exec_opt,   // Options for execution
    int comp_expect,// Expected result for compilation
    int exec_expect // Expected result for execution
    ) {
    
    ri_regex_t regex = NULL;
    const char *log = NULL;
    int ovector[OVECCOUNT] = {0};
    int ret = 0;

    fprintf(stderr, 
            "TEST START\n\t"                      
            "pattern(%s)\n\t"                    
            "subject(%s)\n\t"                    
            "compile option(%d)\n\t"              
            "exection option(%d)\n\t"             
            "expected compilation result(%d)\n\t" 
            "expected execution result(%d)\n", 
            pattern_str, 
            subject_str, 
            comp_opt, 
            exec_opt,
            comp_expect,
            exec_expect);
    ri_set_log_level(RI_LOG_DEBUG);

    ret = ri_create(&regex, pattern_str, comp_opt, NULL,  NULL, &log);
    if (ret != comp_expect) {
        fprintf(stderr, "Unexpected compilation result: expect(%d) get(%d)\n", comp_expect, ret);
        goto test_error;

    // Get compilation error as expected.
    } else if (ret != RI_SUCCESS) {
        goto test_pass;
    }

    //if (ret != RI_SUCCESS) {
    //    fprintf(stderr, "%s\n", log);
    //    goto test_error;
    //}

    ret = ri_exec(regex, subject_str, strlen(subject_str), 0, exec_opt, NULL, ovector, OVECCOUNT, NULL);
    if (ret < 0) {
        fprintf(stderr, "%s\n", ri_get_error_msg(ret));
    }

    if (ret != exec_expect) {
        fprintf(stderr, "Unexpected execution result: expect(%d) get(%d)\n", exec_expect, ret);
        goto test_error;
    }

test_pass:
    if(regex != NULL) {
        ri_free(regex);
        regex = NULL;
    }
    fprintf(stderr, "TEST PASS\n");
    return 0;

test_error:
    if (regex) {
        ri_free(regex);
    }
    fprintf(stderr, "TEST FAIL\n");
    return -1;
}

int main() {
    ri_init();

    // Normal match
    if (test("abc", "abc", RI_COMP_DEFAULT, RI_EXEC_DEFAULT, RI_SUCCESS, 1) != 0) {
        goto exit_error;
    }
    
    // Normal un-match
    if (test("def", "abc", RI_COMP_DEFAULT, RI_EXEC_DEFAULT, RI_SUCCESS, -1) != 0) {
        goto exit_error;
    }

    // Compile error
    if (test("{abc(:", "abc", RI_COMP_DEFAULT, RI_EXEC_DEFAULT, -4353, -1) != 0) {
        goto exit_error;
    }

    // Large match
    if (test("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 
        RI_COMP_DEFAULT, 
        RI_EXEC_DEFAULT,
        RI_SUCCESS,
        1) != 0) {
        goto exit_error;
    }

    // A challenging case for PCRE
    if (test("a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?aaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 
        RI_COMP_DEFAULT, 
        RI_EXEC_DEFAULT,
        RI_SUCCESS,
        1) != 0) {
        goto exit_error;
    }

    // Multi-line support
    if (test("b$", "b\nb", RI_COMP_MULTILINE, RI_EXEC_DEFAULT, RI_SUCCESS, 1) != 0) {
        goto exit_error;
    }

    // Caseless support
    if (test("ABC", "abc", RI_COMP_CASELESS, RI_EXEC_DEFAULT, RI_SUCCESS, 1) != 0) {
        goto exit_error;
    }

    // Capture groups
    if (test("(a+b)(c+d)(e+f)", 
        "abcdef", 
        RI_COMP_DEFAULT, 
        RI_EXEC_DEFAULT,
        RI_SUCCESS,
        4) != 0) {
        goto exit_error;
    }

    // No enough space to store capture groups
    if (test("(a?)(b?)(c?)(d?)(e?)(f?)(g?)(h?)(i?)(j?)(k?)", 
        "abcdefghijk", 
        RI_COMP_DEFAULT, 
        RI_EXEC_DEFAULT,
        RI_SUCCESS,
        0) != 0) {
        goto exit_error;
    }    

    // Not empty match
    if (test("a?b?c?d?e?", 
        "ffff", 
        RI_COMP_DEFAULT, 
        RI_EXEC_NOTEMPTY,
        RI_SUCCESS,
        -1) != 0) {
        goto exit_error;
    }       

    ri_release();
    return 0;

exit_error:
    ri_release();
    return -1;
}
