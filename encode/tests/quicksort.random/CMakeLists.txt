cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(LENGTH 100)
set(REPETITIONS 10)
BUILD_TEST_CASE(quicksort.random quicksort.random.veri)
BUILD_TEST_CONFIG(quicksort.random quicksort.random.veri "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.random.veri.cfg)

set(LENGTH ${LENGTH_PERF})
set(REPETITIONS 100)
BUILD_TEST_CASE(quicksort.random quicksort.random.perf)
BUILD_TEST_CONFIG(quicksort.random quicksort.random.perf "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.random.perf.cfg)

set(LENGTH 10)
set(REPETITIONS 1)
BUILD_TEST_CASE(quicksort.random quicksort.random.cover)
BUILD_TEST_CONFIG(quicksort.random quicksort.random.cover "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.random.cover.cfg)

unset(LENGTH)
unset(REPETITIONS)
