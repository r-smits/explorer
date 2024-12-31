Explorer: A toy real-time raytracer / renderer.

System requirements:
- clangd, c++17 or newer
- Build: make && cmake && ninja
- Mac OS with support for Metal v3.1 at a minimum.
- To build, run: ./build.sh in main cloned repository.

Features:
- Render 3D .obj files inc. textures, with light sources.
- Instance acceleration structures for ray-tracing (Metal3 API).
- Naive implementation of ReSTiR, including global illumination (with temporal re-use of samples).
  https://research.nvidia.com/sites/default/files/pubs/2020-07_Spatiotemporal-reservoir-resampling/ReSTIR.pdf
- Bindless setup. No naive binding of buffers / bytes / textures required.
- Resource manager to manage aformentioned bindless setup.
- Per primitive data stored on dedicated heap.

Inputs:
- Key inputs: W, A, S, D to explore the scene with vector camera (using quaternions)
- Key inputs: look left/right with arrow keys.
- Key inputs: T for rotating objects around Y axis.
- 
![restir_showcase](https://github.com/user-attachments/assets/d6c316aa-aa8b-486a-a651-a847b9f02bb3)
