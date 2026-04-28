cmake_minimum_required(VERSION 3.21)

project(MovieBase VERSION 1.0 LANGUAGES CXX)

# ===== C++ стандарт =====
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ===== Qt путь (можно переопределить через -D или переменную окружения) =====
if(NOT DEFINED CMAKE_PREFIX_PATH)
    if(DEFINED ENV{Qt6_DIR})
        get_filename_component(CMAKE_PREFIX_PATH "$ENV{Qt6_DIR}/../../.." ABSOLUTE)
    elseif(DEFINED ENV{QT_DIR})
        get_filename_component(CMAKE_PREFIX_PATH "$ENV{QT_DIR}" ABSOLUTE)
    else()
        set(CMAKE_PREFIX_PATH "C:/Qt/6.9.2/mingw_64")
    endif()
endif()

# ===== Qt =====
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Concurrent)

# qt_standard_project_setup()  # Отключаем, чтобы избежать автоматического включения AUTOMOC для всех целей

# =========================================
# CORE (чистый C++)
# =========================================
add_library(core INTERFACE)
target_include_directories(core INTERFACE core)

# =========================================
# SERVICE (Repository + Service)
# =========================================
add_library(service
    service/movie_service.cpp
    service/movie_service.h
    service/movie_repository.cpp
    service/movie_repository.h
)

target_include_directories(service PUBLIC service)
target_link_libraries(service
    PUBLIC core
    PRIVATE Qt6::Sql
)

# =========================================
# IMPORTER
# =========================================
add_library(importer STATIC
    importer/csv_importer.cpp
    importer/csv_importer.h
)
set_target_properties(importer PROPERTIES AUTOMOC ON)

target_include_directories(importer PUBLIC importer)
target_link_libraries(importer
    PUBLIC core service
    PRIVATE Qt6::Core
)

# =========================================
# UI
# =========================================
qt_add_executable(MovieBase
    app/main.cpp

    ui/main_window.cpp
    ui/main_window.h

    ui/movie_table_model.cpp
    ui/movie_table_model.h
)

set_target_properties(MovieBase PROPERTIES AUTOMOC ON)

target_include_directories(MovieBase PRIVATE ui)

target_link_libraries(MovieBase
    PRIVATE
        Qt6::Widgets
        Qt6::Sql
        Qt6::Concurrent
        core
        service
        importer
)

# =========================================
# Windows deploy
# =========================================
if(WIN32)
    add_custom_command(TARGET MovieBase POST_BUILD
        COMMAND ${Qt6_DIR}/../../../bin/windeployqt
                $<TARGET_FILE:MovieBase>
    )
endif()

# =========================================
# Install
# =========================================
include(GNUInstallDirs)

install(TARGETS MovieBase
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)