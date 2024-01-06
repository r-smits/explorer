[Title] Explorer - A 3D Renderer built with the Metal API [/Title]

![alt text][src/Assets/ReadMe/explorer_1.png]

Explorer allows you to render 3D .obj files inc. textures.
It is written in MSL (Metal Shader Language), c++, objcpp, and a little objective-c.
And of course with NVIM.

![alt text][src/Assets/ReadMe/explorer_2.png]

System requirements:
- clangd, c++17 or newer
- make && cmake
- ninja for faster build times (is written in Rust, you need Cargo to compile)

To compile a binary:
- Run cmake in the root directory of the project.
- Alternatively, use the build script, ./build.sh.

Controls:
- Use the W, A, S, D keys to rotate the .obj file.






