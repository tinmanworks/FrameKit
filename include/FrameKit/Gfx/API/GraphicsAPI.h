#pragma once


namespace FrameKit {
  enum class GraphicsAPI { 
    None   = 0,
    OpenGL = 1, 
    Vulkan = 2, 
    D3D12  = 3, 
    Metal  = 4 
    };

  inline const char* ToString(GraphicsAPI b) {
      switch (b) {
      case GraphicsAPI::None:   return "None";
      case GraphicsAPI::OpenGL: return "OpenGL";
      case GraphicsAPI::Vulkan: return "Vulkan";
      case GraphicsAPI::D3D12:  return "D3D12";
      case GraphicsAPI::Metal:  return "Metal";
      default:                  return "Unknown";
      }
  }
}
