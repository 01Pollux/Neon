#include <WindowPCH.hpp>
#include <Input/Table/InputDataTable.hpp>

namespace Neon::Input
{
    /// <summary>
    /// Convert a glfw system key state to a Neon system key state.
    /// </summary>
    [[nodiscard]] InputSysKeyState KeyStateFromState(
        int Mods)
    {
        InputSysKeyState State;
        if (Mods) [[unlikely]]
        {
            if (Mods & GLFW_MOD_SHIFT)
            {
                State.SetSysControlState(EKeyboardInput::Shift, true);
            }
            if (Mods & GLFW_MOD_CONTROL)
            {
                State.SetSysControlState(EKeyboardInput::Ctrl, true);
            }
            if (Mods & GLFW_MOD_ALT)
            {
                State.SetSysControlState(EKeyboardInput::Alt, true);
            }
            if (Mods & GLFW_MOD_SUPER)
            {
                State.SetSysControlState(EKeyboardInput::Super, true);
            }
            if (Mods & GLFW_MOD_CAPS_LOCK)
            {
                State.SetSysControlState(EKeyboardInput::CapsLock, true);
            }
        }
        return State;
    }

    /// <summary>
    /// Convert a glfw key to a Neon key.
    /// </summary>
    [[nodiscard]] EKeyboardInput KeyboardInputFromPlatform(
        int Key)
    {
        EKeyboardInput Type = EKeyboardInput::None;

        switch (Key)
        {
        case GLFW_KEY_SPACE:
            Type = EKeyboardInput::Space;
            break;
        case GLFW_KEY_APOSTROPHE: /* ' */
            Type = EKeyboardInput::Apostrophe;
            break;
        case GLFW_KEY_COMMA: /* , */
            Type = EKeyboardInput::Comma;
            break;
        case GLFW_KEY_MINUS: /* - */
            Type = EKeyboardInput::Minus;
            break;
        case GLFW_KEY_PERIOD: /* . */
            Type = EKeyboardInput::Period;
            break;
        case GLFW_KEY_SLASH: /* / */
            Type = EKeyboardInput::Slash;
            break;
        case GLFW_KEY_0:
            Type = EKeyboardInput::_0;
            break;
        case GLFW_KEY_1:
            Type = EKeyboardInput::_1;
            break;
        case GLFW_KEY_2:
            Type = EKeyboardInput::_2;
            break;
        case GLFW_KEY_3:
            Type = EKeyboardInput::_3;
            break;
        case GLFW_KEY_4:
            Type = EKeyboardInput::_4;
            break;
        case GLFW_KEY_5:
            Type = EKeyboardInput::_5;
            break;
        case GLFW_KEY_6:
            Type = EKeyboardInput::_6;
            break;
        case GLFW_KEY_7:
            Type = EKeyboardInput::_7;
            break;
        case GLFW_KEY_8:
            Type = EKeyboardInput::_8;
            break;
        case GLFW_KEY_9:
            Type = EKeyboardInput::_9;
            break;
        case GLFW_KEY_SEMICOLON: /* ; */
            Type = EKeyboardInput::Semicolon;
            break;
        case GLFW_KEY_EQUAL: /* = */
            Type = EKeyboardInput::Equal;
            break;
        case GLFW_KEY_A:
            Type = EKeyboardInput::A;
            break;
        case GLFW_KEY_B:
            Type = EKeyboardInput::B;
            break;
        case GLFW_KEY_C:
            Type = EKeyboardInput::C;
            break;
        case GLFW_KEY_D:
            Type = EKeyboardInput::D;
            break;
        case GLFW_KEY_E:
            Type = EKeyboardInput::E;
            break;
        case GLFW_KEY_F:
            Type = EKeyboardInput::F;
            break;
        case GLFW_KEY_G:
            Type = EKeyboardInput::G;
            break;
        case GLFW_KEY_H:
            Type = EKeyboardInput::H;
            break;
        case GLFW_KEY_I:
            Type = EKeyboardInput::I;
            break;
        case GLFW_KEY_J:
            Type = EKeyboardInput::J;
            break;
        case GLFW_KEY_K:
            Type = EKeyboardInput::K;
            break;
        case GLFW_KEY_L:
            Type = EKeyboardInput::L;
            break;
        case GLFW_KEY_M:
            Type = EKeyboardInput::M;
            break;
        case GLFW_KEY_N:
            Type = EKeyboardInput::N;
            break;
        case GLFW_KEY_O:
            Type = EKeyboardInput::O;
            break;
        case GLFW_KEY_P:
            Type = EKeyboardInput::P;
            break;
        case GLFW_KEY_Q:
            Type = EKeyboardInput::Q;
            break;
        case GLFW_KEY_R:
            Type = EKeyboardInput::R;
            break;
        case GLFW_KEY_S:
            Type = EKeyboardInput::S;
            break;
        case GLFW_KEY_T:
            Type = EKeyboardInput::T;
            break;
        case GLFW_KEY_U:
            Type = EKeyboardInput::U;
            break;
        case GLFW_KEY_V:
            break;
            Type = EKeyboardInput::V;
        case GLFW_KEY_W:
            break;
            Type = EKeyboardInput::W;
        case GLFW_KEY_X:
            break;
            Type = EKeyboardInput::X;
        case GLFW_KEY_Y:
            break;
            Type = EKeyboardInput::Y;
        case GLFW_KEY_Z:
            break;
            Type = EKeyboardInput::Z;
        case GLFW_KEY_LEFT_BRACKET: /* [ */
            break;
            Type = EKeyboardInput::LeftBracket;
            break;
        case GLFW_KEY_BACKSLASH: /* \ */
            Type = EKeyboardInput::Backslash;
            break;
        case GLFW_KEY_RIGHT_BRACKET: /* ] */
            Type = EKeyboardInput::RightBracket;
            break;
        case GLFW_KEY_GRAVE_ACCENT: /* ` */
            Type = EKeyboardInput::GraveAccent;
            break;
        case GLFW_KEY_ESCAPE:
            Type = EKeyboardInput::Escape;
            break;
        case GLFW_KEY_ENTER:
            Type = EKeyboardInput::Enter;
            break;
        case GLFW_KEY_TAB:
            Type = EKeyboardInput::Tab;
            break;
        case GLFW_KEY_BACKSPACE:
            Type = EKeyboardInput::Backspace;
            break;
        case GLFW_KEY_INSERT:
            Type = EKeyboardInput::Insert;
            break;
        case GLFW_KEY_DELETE:
            Type = EKeyboardInput::Delete;
            break;
        case GLFW_KEY_RIGHT:
            Type = EKeyboardInput::RightArrow;
            break;
        case GLFW_KEY_LEFT:
            Type = EKeyboardInput::LeftArrow;
            break;
        case GLFW_KEY_DOWN:
            Type = EKeyboardInput::DownArrow;
            break;
        case GLFW_KEY_UP:
            Type = EKeyboardInput::UpArrow;
            break;
        case GLFW_KEY_PAGE_UP:
            Type = EKeyboardInput::PageUp;
            break;
        case GLFW_KEY_PAGE_DOWN:
            Type = EKeyboardInput::PageDown;
            break;
        case GLFW_KEY_HOME:
            Type = EKeyboardInput::Home;
            break;
        case GLFW_KEY_END:
            Type = EKeyboardInput::End;
            break;
        case GLFW_KEY_CAPS_LOCK:
            Type = EKeyboardInput::CapsLock;
            break;
        case GLFW_KEY_SCROLL_LOCK:
            Type = EKeyboardInput::ScrollLock;
            break;
        case GLFW_KEY_NUM_LOCK:
            Type = EKeyboardInput::NumLock;
            break;
        case GLFW_KEY_PRINT_SCREEN:
            Type = EKeyboardInput::PrintScreen;
            break;
        case GLFW_KEY_PAUSE:
            Type = EKeyboardInput::Pause;
            break;
        case GLFW_KEY_F1:
            Type = EKeyboardInput::F1;
            break;
        case GLFW_KEY_F2:
            Type = EKeyboardInput::F2;
            break;
        case GLFW_KEY_F3:
            Type = EKeyboardInput::F3;
            break;
        case GLFW_KEY_F4:
            Type = EKeyboardInput::F4;
            break;
        case GLFW_KEY_F5:
            Type = EKeyboardInput::F5;
            break;
        case GLFW_KEY_F6:
            Type = EKeyboardInput::F6;
            break;
        case GLFW_KEY_F7:
            Type = EKeyboardInput::F7;
            break;
        case GLFW_KEY_F8:
            Type = EKeyboardInput::F8;
            break;
        case GLFW_KEY_F9:
            Type = EKeyboardInput::F9;
            break;
        case GLFW_KEY_F10:
            Type = EKeyboardInput::F10;
            break;
        case GLFW_KEY_F11:
            Type = EKeyboardInput::F11;
            break;
        case GLFW_KEY_F12:
            Type = EKeyboardInput::F12;
            break;
        case GLFW_KEY_F13:
            Type = EKeyboardInput::F13;
            break;
        case GLFW_KEY_F14:
            Type = EKeyboardInput::F14;
            break;
        case GLFW_KEY_F15:
            Type = EKeyboardInput::F15;
            break;
        case GLFW_KEY_F16:
            Type = EKeyboardInput::F16;
            break;
        case GLFW_KEY_F17:
            Type = EKeyboardInput::F17;
            break;
        case GLFW_KEY_F18:
            Type = EKeyboardInput::F18;
            break;
        case GLFW_KEY_F19:
            Type = EKeyboardInput::F19;
            break;
        case GLFW_KEY_F20:
            Type = EKeyboardInput::F20;
            break;
        case GLFW_KEY_F21:
            Type = EKeyboardInput::F21;
            break;
        case GLFW_KEY_F22:
            Type = EKeyboardInput::F22;
            break;
        case GLFW_KEY_F23:
            Type = EKeyboardInput::F23;
            break;
        case GLFW_KEY_F24:
            Type = EKeyboardInput::F24;
            break;
        case GLFW_KEY_F25:
            Type = EKeyboardInput::F25;
            break;
        case GLFW_KEY_KP_0:
            Type = EKeyboardInput::Keypad0;
            break;
        case GLFW_KEY_KP_1:
            Type = EKeyboardInput::Keypad1;
            break;
        case GLFW_KEY_KP_2:
            Type = EKeyboardInput::Keypad2;
            break;
        case GLFW_KEY_KP_3:
            Type = EKeyboardInput::Keypad3;
            break;
        case GLFW_KEY_KP_4:
            Type = EKeyboardInput::Keypad4;
            break;
        case GLFW_KEY_KP_5:
            Type = EKeyboardInput::Keypad5;
            break;
        case GLFW_KEY_KP_6:
            Type = EKeyboardInput::Keypad6;
            break;
        case GLFW_KEY_KP_7:
            Type = EKeyboardInput::Keypad7;
            break;
        case GLFW_KEY_KP_8:
            Type = EKeyboardInput::Keypad8;
            break;
        case GLFW_KEY_KP_9:
            Type = EKeyboardInput::Keypad9;
            break;
        case GLFW_KEY_KP_DECIMAL:
            Type = EKeyboardInput::KeypadDecimal;
            break;
        case GLFW_KEY_KP_DIVIDE:
            Type = EKeyboardInput::KeypadDivide;
            break;
        case GLFW_KEY_KP_MULTIPLY:
            Type = EKeyboardInput::KeypadMultiply;
            break;
        case GLFW_KEY_KP_SUBTRACT:
            Type = EKeyboardInput::KeypadSubtract;
            break;
        case GLFW_KEY_KP_ADD:
            Type = EKeyboardInput::KeypadAdd;
            break;
        case GLFW_KEY_KP_ENTER:
            Type = EKeyboardInput::KeypadEnter;
            break;
        case GLFW_KEY_KP_EQUAL:
            Type = EKeyboardInput::Equal;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            Type = EKeyboardInput::LShift;
            break;
        case GLFW_KEY_LEFT_CONTROL:
            Type = EKeyboardInput::LCtrl;
            break;
        case GLFW_KEY_LEFT_ALT:
            Type = EKeyboardInput::LAlt;
            break;
        case GLFW_KEY_LEFT_SUPER:
            Type = EKeyboardInput::LSuper;
            break;
        case GLFW_KEY_RIGHT_SHIFT:
            Type = EKeyboardInput::RShift;
            break;
        case GLFW_KEY_RIGHT_CONTROL:
            Type = EKeyboardInput::RCtrl;
            break;
        case GLFW_KEY_RIGHT_ALT:
            Type = EKeyboardInput::RAlt;
            break;
        case GLFW_KEY_RIGHT_SUPER:
            Type = EKeyboardInput::RSuper;
            break;
        case GLFW_KEY_MENU:
            Type = EKeyboardInput::Menu;
            break;
        }

        return Type;
    }

    /// <summary>
    /// Convert a glfw mouse button to a Neon mouse input
    /// </summary>
    EMouseInput MouseInputFromPlatform(
        int Key)
    {
        EMouseInput Type = EMouseInput::None;

        switch (Key)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            Type = EMouseInput::Left;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            Type = EMouseInput::Right;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            Type = EMouseInput::Middle;
            break;
        case GLFW_MOUSE_BUTTON_4:
            Type = EMouseInput::Button4;
            break;
        case GLFW_MOUSE_BUTTON_5:
            Type = EMouseInput::Button5;
            break;
        case GLFW_MOUSE_BUTTON_6:
            Type = EMouseInput::Button6;
            break;
        case GLFW_MOUSE_BUTTON_7:
            Type = EMouseInput::Button7;
            break;
        case GLFW_MOUSE_BUTTON_8:
            Type = EMouseInput::Button8;
            break;
        }

        return Type;
    }

    //

    void InputDataTable::PushKey(
        int Key,
        int ScanCode,
        int Action,
        int Mods)
    {
        auto InputType = KeyboardInputFromPlatform(Key);
        if (InputType != EKeyboardInput::None) [[unlikely]]
        {
            InputAction::BindType ActionType;
            InputAxis::BindType   AxisType;

            switch (Action)
            {
            case GLFW_PRESS:
                ActionType = InputAction::BindType::Press;
                AxisType   = InputAxis::BindType::Press;
                break;
            case GLFW_RELEASE:
                ActionType = InputAction::BindType::Release;
                AxisType   = InputAxis::BindType::Release;
                break;
            case GLFW_REPEAT:
                ActionType = InputAction::BindType::Tick;
                AxisType   = InputAxis::BindType::Tick;
                break;
            default:
                return;
            }

            auto State = KeyStateFromState(Mods);
            m_EventQueue.QueueData<InputActionDataEvent>(State, InputType, ActionType);
            m_EventQueue.QueueData<InputAxisDataEvent>(InputType, AxisType);
        }
    }

    void InputDataTable::PushMouseInput(
        int Key,
        int Action,
        int Mods)
    {
        auto Type = MouseInputFromPlatform(Key);

        if (Type != EMouseInput::None)
        {
            auto State = KeyStateFromState(Mods);

            if (Action == GLFW_RELEASE)
            {
                static auto Before = std::chrono::system_clock::now();

                auto   Now  = std::chrono::system_clock::now();
                double Diff = std::chrono::duration<double, std::milli>(Now - Before).count();
                Before      = Now;

                if (Diff > 10 && Diff < 200)
                {
                    Action = GLFW_REPEAT;
                }
            }

            InputMouse::BindType BindType;
            switch (Action)
            {
            case GLFW_PRESS:
                BindType = InputMouse::BindType::Click;
                break;
            case GLFW_RELEASE:
                BindType = InputMouse::BindType::Release;
                break;
            case GLFW_REPEAT:
                BindType = InputMouse::BindType::DoubleClick;
                break;
            default:
                return;
            }

            m_EventQueue.QueueData<InputMouseDataEvent>(Vector2{}, State, Type, BindType);
        }
    }

    void InputDataTable::PushMouseMove(
        double X,
        double Y)
    {
        m_EventQueue.QueueData<InputMouseDataEvent>(Vector2{ float(X), float(Y) }, InputSysKeyState{}, EMouseInput::None, InputMouse::BindType::Move);
    }

    void InputDataTable::ClearState()
    {
        m_EventQueue.ClearState();
    }

    //

    void InputDataTable::ProcessInputs()
    {
        m_EventQueue.Dispatch(this);
    }

    Ptr<InputActionTable> InputDataTable::LoadActionTable(
        const StringU8& ActionName)
    {
        auto& Action = m_InputActionMap[ActionName];
        if (!Action)
        {
            Action.reset(NEON_NEW InputActionTable);
        }
        return Action;
    }

    Ptr<InputActionTable> InputDataTable::GetActionTable(
        const StringU8& ActionName) const
    {
        auto Iter = m_InputActionMap.find(ActionName);
        return Iter == m_InputActionMap.end() ? nullptr : Iter->second;
    }

    void InputDataTable::RemoveActionTable(
        const StringU8& ActionName)
    {
        m_InputActionMap.erase(ActionName);
    }

    //

    Ptr<InputAxisTable> InputDataTable::LoadAxisTable(
        const StringU8& AxisName)
    {
        auto& Axis = m_InputAxisMap[AxisName];
        if (!Axis)
        {
            Axis.reset(NEON_NEW InputAxisTable);
        }
        return Axis;
    }

    Ptr<InputAxisTable> InputDataTable::GetAxisTable(
        const StringU8& AxisName) const
    {
        auto Iter = m_InputAxisMap.find(AxisName);
        return Iter == m_InputAxisMap.end() ? nullptr : Iter->second;
    }

    void InputDataTable::RemoveAxisTable(
        const StringU8& AxisName)
    {
        m_InputAxisMap.erase(AxisName);
    }

    //

    Ptr<InputMouseTable> InputDataTable::LoadMouseTable(
        const StringU8& MouseName)
    {
        auto& Mouse = m_InputMouseMap[MouseName];
        if (!Mouse)
        {
            Mouse.reset(NEON_NEW InputMouseTable);
        }
        return Mouse;
    }

    Ptr<InputMouseTable> InputDataTable::GetMouseTable(
        const StringU8& MouseName) const
    {
        auto Iter = m_InputMouseMap.find(MouseName);
        return Iter == m_InputMouseMap.end() ? nullptr : Iter->second;
    }

    void InputDataTable::RemoveMouseTable(
        const StringU8& MouseName)
    {
        m_InputMouseMap.erase(MouseName);
    }

    //

    Ptr<InputMouseWheelTable> InputDataTable::LoadMouseWheelTable(
        const StringU8& MouseWheelName)
    {
        auto& MouseWheel = m_InputMouseWheelMap[MouseWheelName];
        if (!MouseWheel)
        {
            MouseWheel.reset(NEON_NEW InputMouseWheelTable);
        }
        return MouseWheel;
    }

    Ptr<InputMouseWheelTable> InputDataTable::GetMouseWheelTable(
        const StringU8& MouseWheelName) const
    {
        auto Iter = m_InputMouseWheelMap.find(MouseWheelName);
        return Iter == m_InputMouseWheelMap.end() ? nullptr : Iter->second;
    }

    void InputDataTable::RemoveMouseWheelTable(
        const StringU8& MouseWheelName)
    {
        m_InputMouseWheelMap.erase(MouseWheelName);
    }

    void InputEventQueue::Dispatch(
        InputDataTable* DataTable)
    {
        auto DispatchAndClearPool = [this](auto& Pool, auto& Tables)
        {
            for (auto& InputData : Pool)
            {
                for (auto& [Name, Table] : Tables)
                {
                    if (!Table->IsEnabled())
                    {
                        continue;
                    }
                    InputData.DispatchInput(Table.get());
                }
            }
            Pool.clear();
        };

        DispatchAndClearPool(GetPool<InputActionDataEvent>(), DataTable->m_InputActionMap);
        DispatchAndClearPool(GetPool<InputAxisDataEvent>(), DataTable->m_InputAxisMap);
        DispatchAndClearPool(GetPool<InputMouseDataEvent>(), DataTable->m_InputMouseMap);
        DispatchAndClearPool(GetPool<InputMouseWheelDataEvent>(), DataTable->m_InputMouseWheelMap);
    }

    void InputEventQueue::ClearState()
    {
        GetPool<InputActionDataEvent>().clear();
        GetPool<InputAxisDataEvent>().clear();
        GetPool<InputMouseDataEvent>().clear();
        GetPool<InputMouseWheelDataEvent>().clear();
    }
} // namespace Neon::Input