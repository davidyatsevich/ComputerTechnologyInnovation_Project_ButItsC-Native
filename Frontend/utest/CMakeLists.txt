# Unit tests for the Frontend, written against the QtTest framework.
#
# main.cpp (which owns int main / QApplication::exec()) is deliberately
# excluded here - each QtTest executable supplies its own main via
# QTEST_MAIN, and Qt Widgets tests run headless via the "offscreen"
# platform plugin (set below) so they work without a real display.

find_package(Qt6 COMPONENTS Test REQUIRED)

set(CMAKE_AUTOMOC ON)

set(FRONTEND_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

function(add_frontend_qtest name)
    add_executable(${name} ${ARGN})
    target_include_directories(${name} PRIVATE
        ${FRONTEND_SRC_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Backend/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Backend/include
    )
    target_link_libraries(${name} PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Charts
        pipeline
    )
    add_test(NAME ${name} COMMAND ${name})
    # No physical display is available in CI/headless environments; the Qt
    # "offscreen" platform plugin lets QWidget-derived code under test
    # still construct and run normally.
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

add_frontend_qtest(tst_filemanager
    tst_filemanager.h tst_filemanager.cpp
    ${FRONTEND_SRC_DIR}/filemanager.cpp
)

add_frontend_qtest(tst_datavisualization
    tst_datavisualization.h tst_datavisualization.cpp
    ${FRONTEND_SRC_DIR}/datavisualization.cpp
)
