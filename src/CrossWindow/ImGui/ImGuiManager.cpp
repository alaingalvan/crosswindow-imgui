#include "ImGuiManager.h"
#include "imgui.h"

#include <algorithm>

namespace
{
std::string str_tolower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}
}

namespace xgfx
{
void ImGuiManager::create()
{
    // Map ImGui inputs to CrossWindow
    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = static_cast<size_t>(xwin::Key::Tab);
    io.KeyMap[ImGuiKey_LeftArrow] = static_cast<size_t>(xwin::Key::Left);
    io.KeyMap[ImGuiKey_RightArrow] = static_cast<size_t>(xwin::Key::Right);
    io.KeyMap[ImGuiKey_UpArrow] = static_cast<size_t>(xwin::Key::Up);
    io.KeyMap[ImGuiKey_DownArrow] = static_cast<size_t>(xwin::Key::Down);
    io.KeyMap[ImGuiKey_PageUp] = static_cast<size_t>(xwin::Key::PgUp);
    io.KeyMap[ImGuiKey_PageDown] = static_cast<size_t>(xwin::Key::PgDn);
    io.KeyMap[ImGuiKey_Home] = static_cast<size_t>(xwin::Key::Home);
    io.KeyMap[ImGuiKey_End] = static_cast<size_t>(xwin::Key::End);
    io.KeyMap[ImGuiKey_Insert] = static_cast<size_t>(xwin::Key::Insert);
    io.KeyMap[ImGuiKey_Delete] = static_cast<size_t>(xwin::Key::Del);
    io.KeyMap[ImGuiKey_Backspace] = static_cast<size_t>(xwin::Key::Back);
    io.KeyMap[ImGuiKey_Space] = static_cast<size_t>(xwin::Key::Space);
    io.KeyMap[ImGuiKey_Enter] = static_cast<size_t>(xwin::Key::Enter);
    io.KeyMap[ImGuiKey_Escape] = static_cast<size_t>(xwin::Key::Escape);

    io.KeyMap[ImGuiKey_LeftCtrl] = static_cast<size_t>(xwin::Key::LControl);
    io.KeyMap[ImGuiKey_LeftShift] = static_cast<size_t>(xwin::Key::LShift);
    io.KeyMap[ImGuiKey_LeftAlt] = static_cast<size_t>(xwin::Key::LAlt);

    io.KeyMap[ImGuiKey_RightCtrl] = static_cast<size_t>(xwin::Key::RControl);
    io.KeyMap[ImGuiKey_RightShift] = static_cast<size_t>(xwin::Key::RShift);
    io.KeyMap[ImGuiKey_RightAlt] = static_cast<size_t>(xwin::Key::RAlt);

    io.KeyMap[ImGuiKey_0] = static_cast<size_t>(xwin::Key::Num0);
    io.KeyMap[ImGuiKey_1] = static_cast<size_t>(xwin::Key::Num1);
    io.KeyMap[ImGuiKey_2] = static_cast<size_t>(xwin::Key::Num2);
    io.KeyMap[ImGuiKey_3] = static_cast<size_t>(xwin::Key::Num3);
    io.KeyMap[ImGuiKey_4] = static_cast<size_t>(xwin::Key::Num4);
    io.KeyMap[ImGuiKey_5] = static_cast<size_t>(xwin::Key::Num5);
    io.KeyMap[ImGuiKey_6] = static_cast<size_t>(xwin::Key::Num6);
    io.KeyMap[ImGuiKey_7] = static_cast<size_t>(xwin::Key::Num7);
    io.KeyMap[ImGuiKey_8] = static_cast<size_t>(xwin::Key::Num8);
    io.KeyMap[ImGuiKey_9] = static_cast<size_t>(xwin::Key::Num9);

    io.KeyMap[ImGuiKey_A] = static_cast<size_t>(xwin::Key::A);
    io.KeyMap[ImGuiKey_A] = static_cast<size_t>(xwin::Key::B);
    io.KeyMap[ImGuiKey_C] = static_cast<size_t>(xwin::Key::C);
    io.KeyMap[ImGuiKey_D] = static_cast<size_t>(xwin::Key::D);
    io.KeyMap[ImGuiKey_E] = static_cast<size_t>(xwin::Key::E);
    io.KeyMap[ImGuiKey_F] = static_cast<size_t>(xwin::Key::F);
    io.KeyMap[ImGuiKey_G] = static_cast<size_t>(xwin::Key::G);
    io.KeyMap[ImGuiKey_H] = static_cast<size_t>(xwin::Key::H);
    io.KeyMap[ImGuiKey_I] = static_cast<size_t>(xwin::Key::I);
    io.KeyMap[ImGuiKey_J] = static_cast<size_t>(xwin::Key::J);
    io.KeyMap[ImGuiKey_K] = static_cast<size_t>(xwin::Key::K);
    io.KeyMap[ImGuiKey_L] = static_cast<size_t>(xwin::Key::L);
    io.KeyMap[ImGuiKey_M] = static_cast<size_t>(xwin::Key::M);
    io.KeyMap[ImGuiKey_N] = static_cast<size_t>(xwin::Key::N);
    io.KeyMap[ImGuiKey_O] = static_cast<size_t>(xwin::Key::O);
    io.KeyMap[ImGuiKey_P] = static_cast<size_t>(xwin::Key::P);
    io.KeyMap[ImGuiKey_Q] = static_cast<size_t>(xwin::Key::Q);
    io.KeyMap[ImGuiKey_R] = static_cast<size_t>(xwin::Key::R);
    io.KeyMap[ImGuiKey_S] = static_cast<size_t>(xwin::Key::S);
    io.KeyMap[ImGuiKey_T] = static_cast<size_t>(xwin::Key::T);
    io.KeyMap[ImGuiKey_U] = static_cast<size_t>(xwin::Key::U);
    io.KeyMap[ImGuiKey_V] = static_cast<size_t>(xwin::Key::V);
    io.KeyMap[ImGuiKey_W] = static_cast<size_t>(xwin::Key::W);
    io.KeyMap[ImGuiKey_X] = static_cast<size_t>(xwin::Key::X);
    io.KeyMap[ImGuiKey_Y] = static_cast<size_t>(xwin::Key::Y);
    io.KeyMap[ImGuiKey_Z] = static_cast<size_t>(xwin::Key::Z);

    io.KeyMap[ImGuiKey_F1] = static_cast<size_t>(xwin::Key::F1);
    io.KeyMap[ImGuiKey_F2] = static_cast<size_t>(xwin::Key::F2);
    io.KeyMap[ImGuiKey_F3] = static_cast<size_t>(xwin::Key::F3);
    io.KeyMap[ImGuiKey_F4] = static_cast<size_t>(xwin::Key::F4);
    io.KeyMap[ImGuiKey_F5] = static_cast<size_t>(xwin::Key::F5);
    io.KeyMap[ImGuiKey_F6] = static_cast<size_t>(xwin::Key::F6);
    io.KeyMap[ImGuiKey_F7] = static_cast<size_t>(xwin::Key::F7);
    io.KeyMap[ImGuiKey_F8] = static_cast<size_t>(xwin::Key::F8);
    io.KeyMap[ImGuiKey_F9] = static_cast<size_t>(xwin::Key::F9);
    io.KeyMap[ImGuiKey_F9] = static_cast<size_t>(xwin::Key::F10);
    io.KeyMap[ImGuiKey_F9] = static_cast<size_t>(xwin::Key::F11);
    io.KeyMap[ImGuiKey_F9] = static_cast<size_t>(xwin::Key::F12);

    io.KeyMap[ImGuiKey_Apostrophe] = static_cast<size_t>(xwin::Key::Apostrophe);
    io.KeyMap[ImGuiKey_Comma] = static_cast<size_t>(xwin::Key::Comma);
    io.KeyMap[ImGuiKey_Minus] = static_cast<size_t>(xwin::Key::Minus);
    io.KeyMap[ImGuiKey_Period] = static_cast<size_t>(xwin::Key::Period);
    io.KeyMap[ImGuiKey_Slash] = static_cast<size_t>(xwin::Key::Slash);
    io.KeyMap[ImGuiKey_Semicolon] = static_cast<size_t>(xwin::Key::Semicolon);
    io.KeyMap[ImGuiKey_Equal] = static_cast<size_t>(xwin::Key::Equals);
    io.KeyMap[ImGuiKey_LeftBracket] = static_cast<size_t>(xwin::Key::LBracket);
    io.KeyMap[ImGuiKey_Backslash] = static_cast<size_t>(xwin::Key::Backslash);
    io.KeyMap[ImGuiKey_RightBracket] = static_cast<size_t>(xwin::Key::RBracket);
    io.KeyMap[ImGuiKey_GraveAccent] = static_cast<size_t>(xwin::Key::Grave);

    io.KeyMap[ImGuiKey_CapsLock] = static_cast<size_t>(xwin::Key::Capital);
    io.KeyMap[ImGuiKey_ScrollLock] = static_cast<size_t>(xwin::Key::Scroll);
    io.KeyMap[ImGuiKey_NumLock] = static_cast<size_t>(xwin::Key::Numlock);

    io.KeyMap[ImGuiKey_Keypad0] = static_cast<size_t>(xwin::Key::Numpad0);
    io.KeyMap[ImGuiKey_Keypad1] = static_cast<size_t>(xwin::Key::Numpad1);
    io.KeyMap[ImGuiKey_Keypad2] = static_cast<size_t>(xwin::Key::Numpad2);
    io.KeyMap[ImGuiKey_Keypad3] = static_cast<size_t>(xwin::Key::Numpad3);
    io.KeyMap[ImGuiKey_Keypad4] = static_cast<size_t>(xwin::Key::Numpad4);
    io.KeyMap[ImGuiKey_Keypad5] = static_cast<size_t>(xwin::Key::Numpad5);
    io.KeyMap[ImGuiKey_Keypad6] = static_cast<size_t>(xwin::Key::Numpad6);
    io.KeyMap[ImGuiKey_Keypad7] = static_cast<size_t>(xwin::Key::Numpad7);
    io.KeyMap[ImGuiKey_Keypad8] = static_cast<size_t>(xwin::Key::Numpad8);
    io.KeyMap[ImGuiKey_Keypad9] = static_cast<size_t>(xwin::Key::Numpad9);

    io.KeyMap[ImGuiKey_Keypad5] = static_cast<size_t>(xwin::Key::Decimal);
    io.KeyMap[ImGuiKey_Keypad6] = static_cast<size_t>(xwin::Key::Divide);
    io.KeyMap[ImGuiKey_Keypad7] = static_cast<size_t>(xwin::Key::Multiply);
    io.KeyMap[ImGuiKey_Keypad8] = static_cast<size_t>(xwin::Key::Subtract);
    io.KeyMap[ImGuiKey_Keypad9] = static_cast<size_t>(xwin::Key::Add);

    io.KeyMap[ImGuiKey_KeypadEnter] =
        static_cast<size_t>(xwin::Key::Numpadenter);

    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
}

void ImGuiManager::updateEvent(xwin::Event e)
{
    ImGuiIO& io = ImGui::GetIO();

    if (e.type == xwin::EventType::Resize)
    {
        if (e.data.resize.resizing)
        {
            return;
        }
        float x = static_cast<float>(e.data.resize.width);
        float y = static_cast<float>(e.data.resize.height);
        io.DisplaySize.x = x / io.DisplayFramebufferScale.x;
        io.DisplaySize.y = y / io.DisplayFramebufferScale.y;
    }
    if (e.type == xwin::EventType::DPI)
    {
        float dpiScale = e.data.dpi.scale;
        io.DisplayFramebufferScale = ImVec2(dpiScale, dpiScale);
    }

    if (e.type == xwin::EventType::MouseInput)
    {
        xwin::MouseInputData& mid = e.data.mouseInput;

        if (mid.state == xwin::ButtonState::Pressed)
        {
            io.MouseDown[static_cast<size_t>(mid.button)] = true;
        }
        else if (mid.state == xwin::ButtonState::Released)
        {
            io.MouseDown[static_cast<size_t>(mid.button)] = false;
        }
    }

    if (e.type == xwin::EventType::MouseMove)
    {
        xwin::MouseMoveData mmd = e.data.mouseMove;

        io.MousePos =
            ImVec2(static_cast<float>(mmd.x) / io.DisplayFramebufferScale.x,
                   static_cast<float>(mmd.y) / io.DisplayFramebufferScale.y);
    }

    if (e.type == xwin::EventType::MouseWheel)
    {
        xwin::MouseWheelData& mwd = e.data.mouseWheel;
        io.MouseWheel += static_cast<float>(mwd.delta);
    }

    if (e.type == xwin::EventType::Keyboard)
    {
        xwin::KeyboardData& kd = e.data.keyboard;
        size_t kid = static_cast<size_t>(kd.key);
        if (kid < 256)
        {
            if (kd.state == xwin::ButtonState::Pressed)
            {
                io.KeysDown[kid] = true;

                std::string typedItem = xwin::convertKeyToString(kd.key);
                if (!kd.modifiers.shift)
                {
                    typedItem = str_tolower(typedItem);
                }
                if (!typedItem.empty())
                {
                    charBuf += typedItem;
                }
            }
            else if (kd.state == xwin::ButtonState::Released)
            {
                io.KeysDown[kid] = false;
            }
        }
    }
}

const std::string& ImGuiManager::getCharacterBuffer() const
{
    return charBuf;
}

void ImGuiManager::clearCharacterBuffer() { charBuf.clear(); }
}
