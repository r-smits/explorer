#pragma once
#include <config.h>
#include <Model/MeshFactory.h>

class Renderer {

    public:
        Renderer(MTL::Device* device);
        ~Renderer();

        virtual void buildPipeline();
        virtual void buildMeshes();
        virtual NS::String* open(std::string path);
        virtual MTL::Library* getLibrary(std::string path);
        virtual MTL::RenderPipelineDescriptor* getRenderPipelineDescriptor(std::string path, std::string vertexFnName, std::string fragmentFnName);
        virtual MTL::VertexDescriptor* getVertexDescriptor();
        virtual MTL::RenderPipelineState* getRenderPipelineState(std::string shaderName, bool serialize);
        virtual MTL::BinaryArchive* createBinaryArchive();
        virtual bool writeBinaryArchive(MTL::RenderPipelineDescriptor*, std::string path);
        virtual bool readBinaryArchive(std::string path);
        virtual NS::String* nsString(std::string str);
        virtual NS::URL* nsUrl(std::string path);
        virtual void draw(MTK::View*);


    private:
        virtual void printError(NS::Error* error);
        MTL::Device* device;
        MTL::CommandQueue* commandQueue;
        MTL::RenderPipelineState *generalPipeline;
        MTL::Buffer* mesh;
        NS::StringEncoding stringEncoding;
        MeshFactory::Mesh quadMesh;
        float t;
};
