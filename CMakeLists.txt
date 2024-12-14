cmake_minimum_required(VERSION 3.28)
project(Explorer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_language(OBJCXX)

message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")

# Library definition
add_executable(EXPLORER 
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/util.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Control/AppDelegate.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Control/AppDelegate.cpp		
	${CMAKE_CURRENT_SOURCE_DIR}/src/View/ViewDelegate.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/View/ViewDelegate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/View/ViewAdapter.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/View/ViewExtender.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/View/ViewExtender.mm
	${CMAKE_CURRENT_SOURCE_DIR}/src/Log/Logger.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Log/Logger.cpp
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
	${CMAKE_CURRENT_SOURCE_DIR}/src/Layer/ImGuiLayer.h	
	${CMAKE_CURRENT_SOURCE_DIR}/src/layer/ImGuiLayer.mm
	${CMAKE_CURRENT_SOURCE_DIR}/src/Layer/BaseLayer.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Layer/BaseLayer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Layer/RayTraceLayer.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Layer/RayTraceLayer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Events/IOState.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Events/IOState.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Buffer.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Types.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Layout.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Descriptor.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Descriptor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/State.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/State.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Draw.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Draw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Heap.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Heap.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Acceleration.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Acceleration.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/DB/Repository.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/DB/Repository.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/DB/ShaderRepository.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/DB/TextureRepository.mm
	${CMAKE_CURRENT_SOURCE_DIR}/src/DB/MeshRepository.mm
	${CMAKE_CURRENT_SOURCE_DIR}/src/Model/Camera.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Model/Camera.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Model/ResourceManager.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Model/ResourceManager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Model/Submesh.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Model/Submesh.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Model/Mesh.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Model/Mesh.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Model/Object.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Model/Object.cpp

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
	"${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/"
	"${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends"
	"${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/misc/cpp/"
)

# find_package(glm CONFIG REQUIRED)
# target_link_libraries(main PRIVATE glm::glm)

file(GLOB IMGUI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/*)
add_library(IMGUI STATIC 
	${IMGUI_SOURCES}
)

set_target_properties(IMGUI PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS ON
)

# Metal cpp library (linker)
target_link_libraries(EXPLORER PUBLIC
	# glm::glm
	IMGUI
	"-framework Metal"
	"-framework MetalKit"
	"-framework AppKit"
	"-framework Foundation"
	"-framework ModelIO"
)

# Add precompiled headers
# Attempted to get to work but does not work currently
#target_precompile_headers(EXPLORER
# 	PUBLIC
# 		${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h
# )

