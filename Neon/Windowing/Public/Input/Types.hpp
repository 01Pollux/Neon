#pragma once

namespace Neon::Input
{
    enum class EInputType
    {
        Action,
        Axis,
        Mouse,
        MouseWheel
    };

    enum class EKeyboardInput : unsigned char
    {
        None,

        LeftArrow,
        RightArrow,
        UpArrow,
        DownArrow,
        PageUp,
        PageDown,
        Home,
        End,
        Insert,
        Delete,
        Backspace,
        Space,
        Enter,
        Tab,
        Escape,

        LCtrl,
        LShift,
        LAlt,
        LSuper,

        RCtrl,
        RShift,
        RAlt,
        RSuper,

        Menu,

        // System Control Keys
        Ctrl,
        Shift,
        Alt,
        Super,
        CapsLock,
        _Begin_SysControl = Ctrl,
        _End_SysContrl    = CapsLock,

        Keypad0,
        Keypad1,
        Keypad2,
        Keypad3,
        Keypad4,
        Keypad5,
        Keypad6,
        Keypad7,
        Keypad8,
        Keypad9,

        _0,
        _1,
        _2,
        _3,
        _4,
        _5,
        _6,
        _7,
        _8,
        _9,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        F25,

        Apostrophe,  // '
        Comma,       // ,
        Minus,       // -
        Period,      // .
        Slash,       // /
        Semicolon,   // ;
        Equal,       // =
        LeftBracket, // [
        Backslash,   // \ 
	    RightBracket,// ]
        GraveAccent, // `

        ScrollLock,
        NumLock,
        PrintScreen,

        Pause,
        KeypadDecimal,
        KeypadDivide,
        KeypadMultiply,
        KeypadSubtract,
        KeypadAdd,
        KeypadEnter,

        _Last = KeypadEnter
    };

    enum class EMouseInput : unsigned char
    {
        None,

        Left,
        Right,
        Middle,

        Button3 = Middle,
        Button4,
        Button5,
        Button6,
        Button7,
        Button8,
    };
} // namespace Neon::Input
