[Title] Explorer - A 3D Renderer built with the Metal API [/Title]

Explorer allows you to render 3D .obj files inc. textures, with light sources (see first picture).
It also supports rudimentary ray-tracing of spheres (see second picture). 
Support for instance acceleration structures was added, this intergrates with the Apple Metal 3 Raytracing API (see third picture).

<img width="986" alt="Screenshot 2024-01-11 at 00 22 29" src="https://github.com/r-smits/explorer/assets/35615011/2813c38e-8732-4dfa-85bd-bfa26e86cf04">
<img width="985" alt="Screenshot 2024-01-18 at 22 56 11" src="https://github.com/r-smits/explorer/assets/35615011/639ff909-517e-4bdb-acc3-fa21a2d56479">
<img <img width="977" alt="Screenshot 2024-01-28 at 20 29 57" src="https://github.com/r-smits/explorer/assets/35615011/fa288350-0306-4ac9-958c-2cca06754c6f">



 
It is written in MSL (Metal Shader Language), c++, objcpp, and a little objective-c.
And of course with NVIM.


System requirements:
- clangd, c++17 or newer
- make && cmake
- ninja for faster build times (is written in Rust, you need Cargo to compile)

To compile a binary:
- Run cmake in the root directory of the project.
- Alternatively, use the build script, ./build.sh.

Controls:
- Use the W, A, S, D keys to rotate the .obj file.


