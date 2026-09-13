cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME 10_Watch)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

set(BSP_SOURCES
        BSP/IIC/iic_hal.c
        BSP/KEY/key.c
        BSP/LCD/lcd.c
        BSP/LCD/lcd_init.c
        BSP/TOUCH/CST816.c
)

set(SYSTEM_SOURCES
        System/delay.c
)

set(GUI_SOURCES
        APP/GUI/ui_set/ui.c
        APP/GUI/ui_set/ui_helpers.c
        APP/GUI/ui_set/components/ui_comp_hook.c
        APP/GUI/ui_set/screens/ui_HomePage.c
)



set(TASKS_SOURCES
        APP/Tasks/Src/user_HardwareInitTask.c
        APP/Tasks/Src/user_LVGLTask.c
        APP/Tasks/Src/user_TasksInit.c
        APP/Tasks/Src/Key_Task.c
)

set(MID_FUNCTION_SOURCES
    Mid_Function/HardWare_DataAccess.c
    Mid_Function/PageManager.c
)

set(LVGL_PORT_SOURCES
        LVGL/Port/lv_port_disp.c
        LVGL/Port/lv_port_indev.c
)

set(LVGL_SRC_DIRS
        LVGL/src
)

set(LVGL_SRC_SOURCES "")
foreach(_d ${LVGL_SRC_DIRS})
    file(GLOB_RECURSE _tmp CONFIGURE_DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/${_d}/*.c)
    list(APPEND LVGL_SRC_SOURCES ${_tmp})
endforeach()

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
        ${BSP_SOURCES}
        ${SYSTEM_SOURCES}
        ${GUI_SOURCES}
        ${TASKS_SOURCES}
        ${LVGL_PORT_SOURCES}
        ${LVGL_SRC_SOURCES}
        ${MID_FUNCTION_SOURCES}
)


# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
        BSP/IIC
        BSP/KEY
        BSP/LCD
        BSP/TOUCH
        System
        APP/Tasks/Inc
        APP/GUI/ui_set
        Mid_Function
        LVGL
        LVGL/Port
        LVGL/src
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# ui_set 的源文件已通过 GUI_SOURCES 直接编入可执行目标，不使用 SquareLine 自带的子工程 CMake