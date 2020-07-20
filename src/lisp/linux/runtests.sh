#!/bin/sh

# Run tests
cd tests
PATH=..:$PATH
for  f in `ls *.l` ; do
    echo $f
    if [ -n "$1" ]; then
	cat $f | lisp > $f.$1
    else
	cat $f | lisp
    fi
    if [ -n "$1" -a "$1" != "reference" ]; then
	diff $f.reference $f.$1
	rm -f $f.$1
    fi
done
