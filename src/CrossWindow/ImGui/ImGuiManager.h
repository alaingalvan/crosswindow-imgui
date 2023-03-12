#pragma once

#include "CrossWindow/Common/Event.h"

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

  protected:
    bool create();
    std::string charBuf;
};
}