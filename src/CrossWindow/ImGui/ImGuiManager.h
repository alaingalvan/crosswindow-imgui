#pragma once

#include "CrossWindow/Common/Event.h"

namespace xgfx
{
class ImGuiManager
{
  public:
    void updateEvent(xwin::Event e);

    const std::string& getCharacterBuffer() const;

  protected:
    bool create();
    std::string charBuf;
};
}