cmake_minimum_required(VERSION 3.16)
project(RiegoAutomatizado VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Opciones de compilación ──────────────────────────────────────────────
option(BUILD_TESTS "Compilar casos de prueba" ON)
option(BUILD_GUI   "Compilar interfaz Qt (requiere Qt6)" OFF)

# ── Incluir directorios ──────────────────────────────────────────────────
include_directories(src)

# ── Ejecutable principal (demo de consola) ───────────────────────────────
add_executable(riego_demo
    main.cpp
)
target_include_directories(riego_demo PRIVATE src)
target_compile_options(riego_demo PRIVATE -O2 -Wall -Wextra)

# ── Tests ────────────────────────────────────────────────────────────────
if(BUILD_TESTS)
    enable_testing()

    foreach(TEST small medium large extreme benchmark)
        add_executable(test_${TEST} tests/test_${TEST}.cpp)
        target_include_directories(test_${TEST} PRIVATE src)
        target_compile_options(test_${TEST} PRIVATE -O2)
        add_test(NAME ${TEST} COMMAND test_${TEST})
    endforeach()
endif()

# ── Interfaz Qt (opcional) ───────────────────────────────────────────────
if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)
    add_executable(riego_gui
        src/gui/main_gui.cpp
        src/gui/mainwindow.cpp
        src/gui/dashboard_widget.cpp
        src/gui/complexity_chart.cpp
    )
    target_link_libraries(riego_gui Qt6::Widgets Qt6::Charts)
    target_include_directories(riego_gui PRIVATE src)
endif()

# ── Mensaje final ────────────────────────────────────────────────────────
message(STATUS "Proyecto: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Compilar tests: ${BUILD_TESTS}")
message(STATUS "Compilar GUI Qt: ${BUILD_GUI}")