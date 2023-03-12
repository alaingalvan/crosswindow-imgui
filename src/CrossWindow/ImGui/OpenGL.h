#pragma once

#include "ImGuiManager.h"
#include "imgui.h"

namespace xgfx
{

/**
 * A fork of ImGUI's OpenGL 3 implementation.
 * imgui/backends/imgui_impl_opengl3.cpp
 *
 * Modified to be more stand alone, removing the need to compile shaders,
 * And the interface simplified.
 */
class OpenGLImGuiManager : public ImGuiManager
{

  public:
    OpenGLImGuiManager();

    ~OpenGLImGuiManager();

    void init();

    void renderDrawData(ImDrawData* drawData);

    bool createFontTexture();

    void destroyFontTexture();

    bool createDeviceObjects();

    void destroyDeviceObjects();

    char mGLSLVersion[32] = "#version 150\n";
    unsigned mFontTexture = 0;
    int mShaderHandle = 0, mVertHandle = 0, mFragHandle = 0;
    int mAttribLocationTex = 0, mAttribLocationProjMtx = 0;
    int mAttribLocationPosition = 0, mAttribLocationUV = 0,
        mAttribLocationColor = 0;
    unsigned int mVboHandle = 0, mElementsHandle = 0;
};
}