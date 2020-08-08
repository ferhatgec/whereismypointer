#!/bin/sh

gcc -std=c99 WhereIsMyPointer.c -o /bin/wimpointer -lX11 && wimpointer
