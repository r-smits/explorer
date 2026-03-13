// #pragma once
// #include <Math/Transformation.h>
// #include <Model/MeshFactory.h>
// #include <View/ViewAdapter.hpp>
// #include <pch.h>
// #include <simd/simd.h>
// #include <Model/Object.h>


// namespace EXP {

// class OrthographicCamera : public Camera {

//     public:
//       OrthographicCamera();
//       ~OrthographicCamera(){};
    
//     public:
//       virtual OrthographicCamera* project() override;
    
//     public:
//       void setLeft(float left);
//       void setRight(float right);
//       void setTop(float top);
//       void setBottom(float bottom);
    
//     private:
//       float left;
//       float right;
//       float top;
//       float bottom;
//         float nearZ;
//         float farZ;
//     };

// class DefaultCamera : public Camera {

//     public:
//       DefaultCamera();
//       ~DefaultCamera(){};
    
//     public:
//       virtual DefaultCamera* project() override;
    
//     public:
//       void setFov(float fov);
//       void setAspectRatio(float aspectRatio);
//       void setNearZ(float nearZ);
//       void setFarZ(float farZ);
    
//     private:
//       float fov;
//       float aspectRatio;
//       float nearZ;
//       float farZ;
//     };
    
// }; // namespace EXP
    