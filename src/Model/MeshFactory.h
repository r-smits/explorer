#pragma once
#include <config.h>


namespace MeshFactory {

    struct Vertex {
        simd::float2 position;
        simd::float3 color;
    };

    struct Mesh {
        MTL::Buffer* vertexBuffer;
        MTL::Buffer* indexBuffer;
    };

    MTL::Buffer* buildTriangle(MTL::Device* device);
    Mesh buildQuad(MTL::Device* device);
}
