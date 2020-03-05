#!/bin/bash

_=${OPAE_LEGACY_ROOT:=..}

declare -i C_CODE_OK=1
declare -i CPP_CODE_OK=1

find_c() {
    find "${OPAE_LEGACY_ROOT}/tools/coreidle" -iname "*.c" -or -iname "*.h"
    find "${OPAE_LEGACY_ROOT}/tools/fpgad" -iname "*.c" -or -iname "*.h"
}

check_c () {
    pushd $(dirname $0) >/dev/null

    CHECKPATCH=checkpatch.pl

    if [ ! -f $CHECKPATCH ]; then
        wget --no-check-certificate https://raw.githubusercontent.com/torvalds/linux/master/scripts/checkpatch.pl
        if [ ! -f $CHECKPATCH ]; then
            echo "Couldn't download checkpatch.pl - please put a copy into the same"
            echo "directory as this script."
            popd >/dev/null
            echo "test-codingstyle-c FAILED"
            return ${C_CODE_OK}
        fi
    fi

    FILES=$(find_c)
    if [ "${FILES}" == "" ]; then
        echo "test-codingstyle-c SKIPPED"
        C_CODE_OK=0
	return ${C_CODE_OK}
    fi

    perl ./$CHECKPATCH --no-tree --no-signoff --terse -f $FILES | grep -v "need consistent spacing" | grep ERROR

    if [ $? -eq 0 ]; then
        echo "test-codingstyle-c FAILED"
    else
        echo "test-codingstyle-c PASSED"
        C_CODE_OK=0
    fi

    popd >/dev/null

    return ${C_CODE_OK}
}

find_cpp() {
    printf ""
}

check_cpp () {
    pushd $(dirname $0) >/dev/null

    FILES=$(find_cpp)
    if [ "${FILES}" == "" ]; then
        echo "test-codingstyle-cpp SKIPPED"
        CPP_CODE_OK=0
	return ${CPP_CODE_OK}
    fi
    clang-format-3.9 -i -style=Google ${FILES}

    if [ x"$(git diff)" != x ]; then
        echo "Coding style check failed. Please fix them based on the following suggestions. You can run clang-format on these files in order to automatically format your code." 
        git --no-pager diff
        echo "The files that need to be fixed are:"
        git diff --name-only
        echo "test-codingstyle-cpp FAILED"
        #git reset --hard HEAD
    else
        echo "test-codingstyle-cpp PASSED"
        CPP_CODE_OK=0
    fi

    popd >/dev/null

    return ${CPP_CODE_OK}
}

check_c
check_cpp

declare -i res=0

if [ ${C_CODE_OK} -ne 0 ]; then
    printf "C FAILED.\n"
    res=1
else
    printf "C PASSED.\n"
fi
if [ ${CPP_CODE_OK} -ne 0 ]; then
    printf "C++ FAILED.\n"
    res=1
else
    printf "C++ PASSED.\n"
fi

if [ ${res} -eq 0 ]; then
    printf "All coding style checks PASSED.\n"
fi
exit ${res}
