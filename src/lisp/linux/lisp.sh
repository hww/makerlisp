#!/bin/sh
if [ -n "$1" ]; then
    cat $1 | lisp
else
    lisp
fi
