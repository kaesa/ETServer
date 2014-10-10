@echo off
REM set up a CVS tree to build when there's no autotools
REM $Revision: 1.1 $
REM $Date: 2012/06/14 23:08:09 $

REM create ca-bundle.h
echo /* This file is generated automatically */ >lib\ca-bundle.h
echo #define CURL_CA_BUNDLE getenv("CURL_CA_BUNDLE") >>lib\ca-bundle.h

REM create hugehelp.c
copy src\hugehelp.c.cvs src\hugehelp.c

REM create Makefile
copy Makefile.dist Makefile