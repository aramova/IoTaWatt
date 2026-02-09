#!/bin/bash
set -e
g++ tests/test_strcmp_ci.cpp -o tests/test_strcmp_ci
./tests/test_strcmp_ci
rm tests/test_strcmp_ci
