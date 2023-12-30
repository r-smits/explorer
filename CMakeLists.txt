cmake_minimum_required(VERSION 3.28)
project(Explorer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_language(OBJCXX)

message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")

# Library definition
add_executable(EXPLORER
	#				${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h
				${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Control/AppDelegate.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Control/AppDelegate.cpp
				${CMAKE_CURRENT_SOURCE_DIR}/src/Control/ViewDelegate.h
				${CMAKE_CURRENT_SOURCE_DIR}/src/Control/ViewDelegate.cpp
				${CMAKE_CURRENT_SOURCE_DIR}/src/Control/ViewAdapter.hpp
				${CMAKE_CURRENT_SOURCE_DIR}/src/Control/ViewExtender.h
				${CMAKE_CURRENT_SOURCE_DIR}/src/Control/ViewExtender.mm
				${CMAKE_CURRENT_SOURCE_DIR}/src/Control/Logger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Control/Logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/View/Renderer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/View/Renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Model/Meshfactory.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Model/MeshFactory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/View/Transformation.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/View/Transformation.cpp
				${CMAKE_CURRENT_SOURCE_DIR}/src/Events/Events.h
				${CMAKE_CURRENT_SOURCE_DIR}/src/Events/KeyEvent.h
				${CMAKE_CURRENT_SOURCE_DIR}/src/Events/MouseEvent.h
)

# Add precompiled headers
# Attempted to get to work but does not work currently
# target_precompile_headers(EXPLORER
# 	PUBLIC
# 		${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h
# )

# Configure compilation variables
set_target_properties(EXPLORER PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
				CXX_EXTENSIONS ON
)

# Metal cpp headers
target_include_directories(EXPLORER PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include"
)

# Metal cpp library (linker)
target_link_libraries(EXPLORER
        "-framework Metal"
        "-framework MetalKit"
        "-framework AppKit"
        "-framework Foundation"
        "-framework QuartzCore"
				"-framework GameController"
)


