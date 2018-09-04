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
    int comp_opt,
    int exec_opt,
    int expect) {
    
    // pattern_str = "a$";
    // subject_str = "b\nb";
    ri_regex_t regex = NULL;
    const char *log = NULL;
    int ovector[OVECCOUNT] = {0};
    int ret = 0;

    // comp_opt = RI_COMP_MULTILINE | RI_COMP_CASELESS;
    fprintf(stderr, "TEST START \n\tpattern(%s)\n\tsubject(%s)\n\tcompile option(%d)\n\texection option(%d)\n\texpect(%d)\n", 
        pattern_str, 
        subject_str, 
        comp_opt, 
        exec_opt,
        expect);
    ri_set_log_level(RI_LOG_DEBUG);

    ret = ri_create(&regex, pattern_str, comp_opt, NULL,  NULL, &log);
    if (ret != RI_SUCCESS) {
        fprintf(stderr, "%s\n", log);
        goto test_error;
    }

    ret = ri_exec(regex, subject_str, strlen(subject_str), 0, exec_opt, NULL, ovector, OVECCOUNT, NULL);
    if (ret < 0) {
        fprintf(stderr, "%s\n", ri_get_error_msg(ret));
    }
    if (ret != expect) {
        fprintf(stderr, "FAIL get(%d)\n", ret);
        goto test_error;
    }
    if(regex != NULL) {
        ri_free(regex);
        regex = NULL;
    }
    fprintf(stderr, "TEST FINISH\n");
    return 0;

test_error:
    if (regex) {
        ri_free(regex);
    }
    fprintf(stderr, "TEST FINISH\n");
    return -1;
}

int main() {
    ri_init();

    // Normal match
    if (test("abc", "abc", RI_COMP_DEFAULT, RI_EXEC_DEFAULT, 1) != 0) {
        goto exit_error;
    }
    
    // Normal un-match
    if (test("def", "abc", RI_COMP_DEFAULT, RI_EXEC_DEFAULT, -1) != 0) {
        goto exit_error;
    }

    // Compile error
    if (test("{abc(:", "abc", RI_COMP_DEFAULT, RI_EXEC_DEFAULT, -1) != -1) {
        goto exit_error;
    }

    // Large match
    if (test("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 
        RI_COMP_DEFAULT, 
        RI_EXEC_DEFAULT,
        1) != 0) {
        goto exit_error;
    }

    // Newline support
    if (test("b$", "b\nb", RI_COMP_MULTILINE | RI_COMP_CASELESS, RI_EXEC_DEFAULT, 1) != 0) {
        goto exit_error;
    }

    ri_release();
    return 0;

exit_error:
    ri_release();
    return -1;
}
