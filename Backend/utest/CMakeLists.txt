# Unit tests for the Backend, written against the QtTest framework.
#
# These tests only need Qt for the test harness (QtTest); the code under
# test (CSVReader, KMeansProcessor, GRUProcessor, Pipeline) is plain
# C++/Eigen with no Qt dependency, so this stays independent from the
# Frontend's Qt Widgets/Charts requirements.

find_package(Qt6 COMPONENTS Test REQUIRED)

set(CMAKE_AUTOMOC ON)

# Helper to declare a QtTest-based test executable + register it with ctest.
function(add_backend_qtest name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE pipeline Qt6::Test)
    target_include_directories(${name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_backend_qtest(tst_csvreader tst_csvreader.h tst_csvreader.cpp)
add_backend_qtest(tst_kmeansprocessor tst_kmeansprocessor.h tst_kmeansprocessor.cpp)
add_backend_qtest(tst_gruprocessor tst_gruprocessor.h tst_gruprocessor.cpp)
add_backend_qtest(tst_pipeline tst_pipeline.h tst_pipeline.cpp)
