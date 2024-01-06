Explorer - A 3D Renderer built with the Metal API

Description:
- Explorer allows you to render 3D .obj files inc. textures.
- It is written in MSL (Metal Shader Language), c++, objcpp, and a little objective-c.
- And of course with NVIM.

System requirements:
- clangd, c++17 or newer
- make && cmake
- ninja for faster build times (is written in Rust, you need Cargo to compile)

To compile a binary:
- Run cmake in the root directory of the project.
- Alternatively, use the build script, ./build.sh.

Controls:
- Use the W, A, S, D keys to rotate the .obj file.

<img width="1004" alt="explorer_1" src="https://github.com/r-smits/explorer/assets/35615011/2169e39b-bdf7-44ab-aad5-627e703f0dad">
<img width="1004" alt="explorer_2" src="https://github.com/r-smits/explorer/assets/35615011/6f8cea80-c7c2-4393-aac1-bd86dbd1d840">


