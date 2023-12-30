#pragma once
#include <AppKit/AppKit.hpp>
#include <Control/Logger.h>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
#include <simd/simd.h>


#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <cmath>

/**
 * This file is meant to be a pre-compiled header.
 *
 * General structure of application
 *
 * Model
 *  - The meshes; the files from which you can get the object you're trying to
 represent by renderer

 * Control
 *  - main.cpp starts a NS::Application;
 *  - Receives input from users, get data from users, passes data to the view
 *  - AppDelegate which owns the view -> it wraps around NS::Application for a
 starting point
 *
 * View
 *  - Renderer -> Owns compilation of Shaders into libraries, serialization
 and-so-forth
 *
 *
 * Called the MVC model
 *
 *
 *
 * According to Cherno
 *
 * Entrypoint
 * Application layer
 * Window layer
 *  -> Input
 *  -> Events
 * Renderer
 *  -> Interfaces with Metal
 * Renderer API
 *  -> To make generic if needed
 * Scripting language
 *  -> To interface with other software
 * Memory systems
 *  -> To ensure we manage memory well
 * Entity-component system
 *  -> To manage all the meshes (ECS)
 *
 *
 *
 *
 *
 *
 *
*/
