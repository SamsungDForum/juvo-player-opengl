#!/bin/bash

commit_id=0x`git rev-parse --short HEAD`

echo "#define VERSION ${commit_id}" > include/version.h
