cmake_minimum_required(VERSION 3.28)
project(Explorer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_language(OBJCXX)

message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")

# Library definition
add_executable(EXPLORER
				${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Control/AppDelegate.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Control/AppDelegate.cpp
				${CMAKE_CURRENT_SOURCE_DIR}/src/View/ViewDelegate.h
				${CMAKE_CURRENT_SOURCE_DIR}/src/View/ViewDelegate.cpp
				${CMAKE_CURRENT_SOURCE_DIR}/src/View/ViewAdapter.hpp
				${CMAKE_CURRENT_SOURCE_DIR}/src/View/ViewExtender.h
				${CMAKE_CURRENT_SOURCE_DIR}/src/View/ViewExtender.mm
				${CMAKE_CURRENT_SOURCE_DIR}/src/Log/Logger.h
				${CMAKE_CURRENT_SOURCE_DIR}/src/Log/Logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/View/Renderer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/View/Renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Model/Meshfactory.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Model/MeshFactory.cpp
				${CMAKE_CURRENT_SOURCE_DIR}/src/Math/Transformation.h
				${CMAKE_CURRENT_SOURCE_DIR}/src/Math/Transformation.cpp
				${CMAKE_CURRENT_SOURCE_DIR}/src/Events/Events.h
				${CMAKE_CURRENT_SOURCE_DIR}/src/Events/KeyEvent.h
				${CMAKE_CURRENT_SOURCE_DIR}/src/Events/MouseEvent.h
				${CMAKE_CURRENT_SOURCE_DIR}/src/Layer/Layer.h
				${CMAKE_CURRENT_SOURCE_DIR}/src/Layer/Layer.cpp
				${CMAKE_CURRENT_SOURCE_DIR}/src/Layer/LayerStack.h
				${CMAKE_CURRENT_SOURCE_DIR}/src/Layer/LayerStack.cpp
)


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
				"-framework Cocoa"
				"-framework CoreData"
)

# Add precompiled headers
# Attempted to get to work but does not work currently
#target_precompile_headers(EXPLORER
# 	PUBLIC
# 		${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h
# )

