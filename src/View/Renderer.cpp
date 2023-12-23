#include <View/Renderer.h>
#include <View/Transformation.h>

Renderer::Renderer(MTL::Device* device): device(device->retain()) {
    this->commandQueue = device->newCommandQueue();
    this->stringEncoding = NS::StringEncoding::UTF8StringEncoding;
    this->buildPipeline();
    this->buildMeshes();
}

Renderer::~Renderer() {
    this->quadMesh.vertexBuffer->release();
    this->quadMesh.indexBuffer->release();
    this->commandQueue->release();
    this->device->release();
    this->generalPipeline->release();
    this->mesh->release();
}

void Renderer::buildPipeline() {
    this->generalPipeline = this->getRenderPipelineState("General", true);
}

void Renderer::buildMeshes() {
    this->mesh = MeshFactory::buildTriangle(this->device);
    this->quadMesh = MeshFactory::buildQuad(this->device);
}

NS::String* Renderer::open(std::string path) {
    std::ifstream file;
    file.open(path);
    std::stringstream reader;
    reader << file.rdbuf();
    std::string rawString = reader.str();
    file.close();
    return NS::String::string(rawString.c_str(), this->stringEncoding);
}

MTL::Library* Renderer::getLibrary(std::string path) {
    std::cout << "Creating library from " << path << " ..." << std::endl;
    NS::Error* error = nullptr;
    MTL::CompileOptions* options = nullptr;
    MTL::Library* library = device->newLibrary(open(path), options, &error);
    if (!library) this->printError(error);
    error->release();
    options->release();
    return library;
}

MTL::RenderPipelineDescriptor* Renderer::getRenderPipelineDescriptor(
    std::string path, 
    std::string vertexFnName, 
    std::string fragmentFnName) 
{
    MTL::Library* library = this->getLibrary(path);
    MTL::RenderPipelineDescriptor* renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    renderPipelineDescriptor->setVertexFunction(library->newFunction(nsString(vertexFnName)));
    renderPipelineDescriptor->setFragmentFunction(library->newFunction(nsString(fragmentFnName)));
    renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
    renderPipelineDescriptor->setVertexDescriptor(this->getVertexDescriptor());
    library->release();
    return renderPipelineDescriptor;
}

MTL::VertexDescriptor* Renderer::getVertexDescriptor() {
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

    auto positionDescriptor = vertexDescriptor->attributes()->object(0);
    positionDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat2);
    positionDescriptor->setOffset(0);
    positionDescriptor->setBufferIndex(0);

    auto colorDescriptor = vertexDescriptor->attributes()->object(1);
    colorDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
    colorDescriptor->setOffset(4 * sizeof(float));
    colorDescriptor->setBufferIndex(0);

    auto layoutDescriptor = vertexDescriptor->layouts()->object(0);
    layoutDescriptor->setStride(8 * sizeof(float));

    return vertexDescriptor;
}

MTL::RenderPipelineState* Renderer::getRenderPipelineState(
    std::string shaderName,
    bool serialize
) {
    std::string basePath = "/Users/ramonsmits/Code/Explorer/src/Shaders/";
    MTL::RenderPipelineDescriptor* descriptor = this->getRenderPipelineDescriptor(
        basePath + shaderName + ".metal",
        "vertexMain" + shaderName,
        "fragmentMain" + shaderName
    );
    if (serialize) this->writeBinaryArchive(descriptor, basePath + shaderName + ".metallib");
    NS::Error* newPipelineStateError = nullptr;
    MTL::RenderPipelineState* state = device->newRenderPipelineState(descriptor, &newPipelineStateError);
    if (!state) this->printError(newPipelineStateError);
    newPipelineStateError->release();
    descriptor->release();
    return state;
}

MTL::BinaryArchive* Renderer::createBinaryArchive() {
    NS::Error* error = nullptr;
    MTL::BinaryArchiveDescriptor* binaryArchiveDescriptor = MTL::BinaryArchiveDescriptor::alloc()->init();
    MTL::BinaryArchive* binaryArchive = device->newBinaryArchive(binaryArchiveDescriptor, &error);
    error->release();
    return binaryArchive;
}

bool Renderer::writeBinaryArchive(MTL::RenderPipelineDescriptor* renderPipelineDescriptor, std::string path) {
    std::cout << "Serializing binary to " << path << " ..." << std::endl;
    NS::Error* error = nullptr;
    MTL::BinaryArchive* mlib = this->createBinaryArchive();
    mlib->addRenderPipelineFunctions(renderPipelineDescriptor, &error);
    bool result = mlib->serializeToURL(this->nsUrl(path), &error);
    if (!result) this->printError(error);
    error->release();
    mlib->release();
    return result;
}

bool Renderer::readBinaryArchive(std::string path) {
    return true;
}

NS::String* Renderer::nsString(std::string str) {
    return NS::String::string(str.c_str(), this->stringEncoding);
}

NS::URL* Renderer::nsUrl(std::string path) {
    return NS::URL::alloc()->initFileURLWithPath(nsString(path));
}

void Renderer::printError(NS::Error* error) {
    std::cout << error->localizedDescription()->utf8String() << std::endl;
}

void Renderer::draw(MTK::View* view) {

    t += 0.0005f;
    if (t > 360) t-=360.0f;

    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    MTL::CommandBuffer* CommandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPass = view->currentRenderPassDescriptor();

    MTL::RenderCommandEncoder* encoder = CommandBuffer->renderCommandEncoder(renderPass);

    encoder->setRenderPipelineState(this->generalPipeline);

    simd::float4x4 identityMatrix = Transformation::identity();
    encoder->setVertexBytes(&identityMatrix, sizeof(simd::float4x4), 1);
    encoder->setVertexBuffer(this->quadMesh.vertexBuffer, 0, 0);

    encoder->drawIndexedPrimitives(
        MTL::PrimitiveType::PrimitiveTypeTriangle,
        NS::UInteger(6),
        MTL::IndexType::IndexTypeUInt16,
        this->quadMesh.indexBuffer,
        NS::UInteger(0),
        NS::UInteger(1)
    );

    encoder->setVertexBuffer(this->mesh, 0, 0);

    simd::float4x4 m = Transformation::translation({0.25, 0.25, 0}) * Transformation::zRotation(t) * Transformation::scale(0.25);
    encoder->setVertexBytes(&m, sizeof(simd::float4x4), 1);
    encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(3));
    
    encoder->endEncoding();
    
    CommandBuffer->presentDrawable(view->currentDrawable());
    CommandBuffer->commit();

    pool->release();
}



