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

        LeftCtrl,
        LeftShift,
        LeftAlt,
        RightCtrl,
        RightShift,
        RightAlt,
        _Begin_SysControl = LeftCtrl,
        _End_SysContrl    = RightAlt,

        LeftSuper,
        RightSuper,
        Menu,

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

        CapsLock,
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
        X1,
        X2,
    };
} // namespace Neon::Input
