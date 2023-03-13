#pragma once

#include "CrossWindow/Common/Event.h"
#include <string>

namespace xgfx
{
class ImGuiManager
{
  public:
    // Process a CrossWindow event with ImGui.
    void updateEvent(xwin::Event e);

    // Get the character buffer for the current ImGui context. Useful for
    // getting written strings.
    const std::string& getCharacterBuffer() const;

    void clearCharacterBuffer();

  protected:
    void create();
    std::string charBuf;
};
}