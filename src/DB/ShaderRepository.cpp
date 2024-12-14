#include <DB/Repository.hpp>


NS::String* Repository::Shaders::open(std::string path) {
  std::ifstream file;
  file.open(path);
  std::stringstream reader;
  reader << file.rdbuf();
  std::string rawString = reader.str();
  file.close();
  return NS::String::string(rawString.c_str(), NS::StringEncoding::UTF8StringEncoding);
}

MTL::Library* Repository::Shaders::readLibrary(MTL::Device* device, std::string path) {
  DEBUG("Loading: " + path + ".metal" + " ... ");
  NS::Error* error = nullptr;
  MTL::CompileOptions* options = nullptr;
  if (!device) WARN("No device found!");
  MTL::Library* library = device->newLibrary(open(path + ".metal"), options, &error);
  if (!library) EXP::printError(error);
  error->release();
  options->release();
  return library;
}

MTL::BinaryArchive* Repository::Shaders::createBinaryArchive(MTL::Device* device) {
  NS::Error* error = nullptr;
  MTL::BinaryArchiveDescriptor* descriptor = MTL::BinaryArchiveDescriptor::alloc()->init();
  MTL::BinaryArchive* archive = device->newBinaryArchive(descriptor, &error);
  if (error) EXP::printError(error);
  error->release();
  return archive;
}

bool Repository::Shaders::write(MTL::Device* device,
    MTL::RenderPipelineDescriptor* pipelineDescriptor, std::string path
) {
  DEBUG("Serializing binary ...");
  NS::Error* error = nullptr;
  MTL::BinaryArchiveDescriptor* descriptor = MTL::BinaryArchiveDescriptor::alloc()->init();
  MTL::BinaryArchive* archive = device->newBinaryArchive(descriptor, &error);
  archive->setLabel(EXP::nsString("General shader lib"));

  pipelineDescriptor->setSupportAddingVertexBinaryFunctions(true);
  pipelineDescriptor->setSupportAddingFragmentBinaryFunctions(true);
  archive->addRenderPipelineFunctions(pipelineDescriptor, &error);
  if (error) EXP::printError(error);

  bool result = archive->serializeToURL(EXP::nsUrl(path + ".metallib"), &error);
  if (error) EXP::printError(error);

  error->release();
  archive->release();
  return result;
}

MTL::RenderPipelineDescriptor* Repository::Shaders::read(MTL::Device* device, std::string path) {
  DEBUG("Reading compiled shader from path ...");
  NS::Error* error = nullptr;
  MTL::BinaryArchiveDescriptor* archiveDescriptor = MTL::BinaryArchiveDescriptor::alloc()->init();
  archiveDescriptor->setUrl(
      NS::URL::alloc()->initFileURLWithPath(EXP::nsString(path + ".metallib"))
  );
  MTL::BinaryArchive* archive = device->newBinaryArchive(archiveDescriptor, &error);
  if (error) EXP::printError(error);

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
