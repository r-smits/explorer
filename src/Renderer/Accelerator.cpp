



/**
- (NSArray<id<MTLAccelerationStructure>> *)
    allocateAndBuildAccelerationStructuresWithDescriptors:
        (NSArray<MTLAccelerationStructureDescriptor *> *)descriptors
                                                     heap:(id<MTLHeap>)heap
                                     maxScratchBufferSize:(size_t)maxScratchSize
                                              signalEvent:(id<MTLEvent>)event
    NS_AVAILABLE(13, 16) {
  NSAssert(heap, @"Heap argument is required");

  NSMutableArray<id<MTLAccelerationStructure>> *accelStructures =
      [NSMutableArray arrayWithCapacity:descriptors.count];

  id<MTLBuffer> scratch =
      [_device newBufferWithLength:maxScratchSize
                           options:MTLResourceStorageModePrivate];
  id<MTLCommandBuffer> cmd = [_commandQueue commandBuffer];
  id<MTLAccelerationStructureCommandEncoder> enc =
      [cmd accelerationStructureCommandEncoder];

  for (MTLPrimitiveAccelerationStructureDescriptor *descriptor in descriptors) {
    MTLSizeAndAlign sizes = [_device
        heapAccelerationStructureSizeAndAlignWithDescriptor:descriptor];
    id<MTLAccelerationStructure> accelStructure =
        [heap newAccelerationStructureWithSize:sizes.size];
    [enc buildAccelerationStructure:accelStructure
                         descriptor:descriptor
                      scratchBuffer:scratch
                scratchBufferOffset:0];
    [accelStructures addObject:accelStructure];
  }

  [enc endEncoding];
  [cmd encodeSignalEvent:event value:kPrimitiveAccelerationStructureBuild];
  [cmd commit];

  return accelStructures;
}
**/

