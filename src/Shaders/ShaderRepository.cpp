#include <Shaders/ShaderRepository.h>

Explorer::ShaderRepository::ShaderRepository(MTL::Device* device, std::string basePath)
    : device(device->retain()), basePath(basePath),
      encoding(NS::StringEncoding::UTF8StringEncoding) {}

NS::String* Explorer::ShaderRepository::open(std::string path) {
  std::ifstream file;
  file.open(path);
  std::stringstream reader;
  reader << file.rdbuf();
  std::string rawString = reader.str();
  file.close();
  return NS::String::string(rawString.c_str(), encoding);
}

MTL::Library* Explorer::ShaderRepository::getLibrary(std::string shaderName) {
  DEBUG("Loading: " + shaderName + ".metal" + " ... ");
  NS::Error* error = nullptr;
  MTL::CompileOptions* options = nullptr;
  if (!device) WARN("No device found!");
  MTL::Library* library =
      device->newLibrary(open(basePath + shaderName + ".metal"), options, &error);
  if (!library) printError(error);
  error->release();
  options->release();
  return library;
}

MTL::BinaryArchive* Explorer::ShaderRepository::createBinaryArchive() {
  NS::Error* error = nullptr;
  MTL::BinaryArchiveDescriptor* descriptor = MTL::BinaryArchiveDescriptor::alloc()->init();
  MTL::BinaryArchive* archive = device->newBinaryArchive(descriptor, &error);
  if (error) printError(error);
  error->release();
  return archive;
}

bool Explorer::ShaderRepository::write(
    MTL::RenderPipelineDescriptor* pipelineDescriptor, std::string shaderName
) {
  DEBUG("Serializing binary ...");
  NS::Error* error = nullptr;
  MTL::BinaryArchiveDescriptor* descriptor = MTL::BinaryArchiveDescriptor::alloc()->init();
  MTL::BinaryArchive* archive = device->newBinaryArchive(descriptor, &error);
  archive->setLabel(nsString("General shader lib"));

  pipelineDescriptor->setSupportAddingVertexBinaryFunctions(true);
  pipelineDescriptor->setSupportAddingFragmentBinaryFunctions(true);
  archive->addRenderPipelineFunctions(pipelineDescriptor, &error);
  if (error) printError(error);

  bool result = archive->serializeToURL(nsUrl(basePath + shaderName + ".metallib"), &error);
  if (error) printError(error);

  error->release();
  archive->release();
  return result;
}

MTL::RenderPipelineDescriptor* Explorer::ShaderRepository::read(std::string shaderName) {
  DEBUG("Reading compiled shader from path ...");
  NS::Error* error = nullptr;
  MTL::BinaryArchiveDescriptor* archiveDescriptor = MTL::BinaryArchiveDescriptor::alloc()->init();
  archiveDescriptor->setUrl(
      NS::URL::alloc()->initFileURLWithPath(nsString(basePath + shaderName + ".metallib"))
  );
  MTL::BinaryArchive* archive = device->newBinaryArchive(archiveDescriptor, &error);
  if (error) printError(error);

  std::stringstream ss0;
  ss0 << archiveDescriptor->debugDescription()->utf8String();
  DEBUG(ss0.str());

  DEBUG("Attempt to generate render pipeline descriptor ...");
  MTL::RenderPipelineDescriptor* pipelineDescriptor =
      MTL::RenderPipelineDescriptor::alloc()->init();

  DEBUG("Check support for adding Vertex Binary Functions ...");
  std::stringstream ss1;
  pipelineDescriptor->setSupportAddingVertexBinaryFunctions(true);
  pipelineDescriptor->setSupportAddingFragmentBinaryFunctions(true);
  ss1 << "Vertex bin Fn: " << pipelineDescriptor->supportAddingVertexBinaryFunctions()
      << " Fragment bin Fn: " << pipelineDescriptor->supportAddingFragmentBinaryFunctions();
  DEBUG(ss1.str());

  DEBUG("Setting binary archive to descriptor ...");
  const NS::Array* binArray = NS::Array::alloc()->init()->array(archive);
  pipelineDescriptor->setBinaryArchives(binArray);

  ss1 << pipelineDescriptor->debugDescription()->utf8String();
  DEBUG(ss1.str());

  return pipelineDescriptor;
}


