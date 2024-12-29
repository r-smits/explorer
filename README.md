Explorer: A toy real-time raytracer / renderer.

System requirements:
- clangd, c++17 or newer
- Build: make && cmake && ninja
- Mac OS with support for Metal v3.1 at a minimum.
- To build, run: ./build.sh in main cloned repository.

Features:
- Render 3D .obj files inc. textures, with light sources.
- Instance acceleration structures for ray-tracing (Metal3 API).
- Naive implementation of ReSTiR (without temporal or spacial re-use of samples at this point)
  https://research.nvidia.com/sites/default/files/pubs/2020-07_Spatiotemporal-reservoir-resampling/ReSTIR.pdf
- Bindless setup. No naive binding of buffers / bytes / textures required.
- Resource manager to manage aformentioned bindless setup.
- Per primitive data stored on dedicated heap.

Inputs:
- Key inputs: W, A, S, D to explore the scene with vector camera (using quaternions)
- Key inputs: look left/right with arrow keys.
- Key inputs: T for rotating objects around Y axis.

<img width="987" alt="Screenshot 2024-12-29 at 12 50 27" src="https://github.com/user-attachments/assets/7d64fcf1-1fd6-43e4-b1bf-cb0be87ad984" />
<img width="985" alt="Screenshot 2024-12-14 at 18 43 21" src="https://github.com/user-attachments/assets/1d444621-15a9-4f8f-895a-d24749a82918" />
<img width="986" alt="Screenshot 2024-01-11 at 00 22 29" src="https://github.com/r-smits/explorer/assets/35615011/2813c38e-8732-4dfa-85bd-bfa26e86cf04">
<img width="985" alt="Screenshot 2024-01-18 at 22 56 11" src="https://github.com/r-smits/explorer/assets/35615011/639ff909-517e-4bdb-acc3-fa21a2d56479">
<img <img width="977" alt="Screenshot 2024-01-28 at 20 29 57" src="https://github.com/r-smits/explorer/assets/35615011/fa288350-0306-4ac9-958c-2cca06754c6f">


To compile a binary:
- Run cmake in the root directory of the project.
- Alternatively, use the build script, ./build.sh.

Controls:
- Use the W, A, S, D keys to rotate the .obj file.


