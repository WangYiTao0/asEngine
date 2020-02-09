#pragma once

namespace as
{
	typedef enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space               = 32,
		Apostrophe          = 39, /* ' */
		Comma               = 44, /* , */
		Minus               = 45, /* - */
		Period              = 46, /* . */
		Slash               = 47, /* / */

		D0                  = 48, /* 0 */
		D1                  = 49, /* 1 */
		D2                  = 50, /* 2 */
		D3                  = 51, /* 3 */
		D4                  = 52, /* 4 */
		D5                  = 53, /* 5 */
		D6                  = 54, /* 6 */
		D7                  = 55, /* 7 */
		D8                  = 56, /* 8 */
		D9                  = 57, /* 9 */

		Semicolon           = 59, /* ; */
		Equal               = 61, /* = */

		A                   = 65,
		B                   = 66,
		C                   = 67,
		D                   = 68,
		E                   = 69,
		F                   = 70,
		G                   = 71,
		H                   = 72,
		I                   = 73,
		J                   = 74,
		K                   = 75,
		L                   = 76,
		M                   = 77,
		N                   = 78,
		O                   = 79,
		P                   = 80,
		Q                   = 81,
		R                   = 82,
		S                   = 83,
		T                   = 84,
		U                   = 85,
		V                   = 86,
		W                   = 87,
		X                   = 88,
		Y                   = 89,
		Z                   = 90,

		LeftBracket         = 91,  /* [ */
		Backslash           = 92,  /* \ */
		RightBracket        = 93,  /* ] */
		GraveAccent         = 96,  /* ` */

		World1              = 161, /* non-US #1 */
		World2              = 162, /* non-US #2 */

		/* Function keys */
		Escape              = 256,
		Enter               = 257,
		Tab                 = 258,
		Backspace           = 259,
		Insert              = 260,
		Delete              = 261,
		Right               = 262,
		Left                = 263,
		Down                = 264,
		Up                  = 265,
		PageUp              = 266,
		PageDown            = 267,
		Home                = 268,
		End                 = 269,
		CapsLock            = 280,
		ScrollLock          = 281,
		NumLock             = 282,
		PrintScreen         = 283,
		Pause               = 284,
		F1                  = 290,
		F2                  = 291,
		F3                  = 292,
		F4                  = 293,
		F5                  = 294,
		F6                  = 295,
		F7                  = 296,
		F8                  = 297,
		F9                  = 298,
		F10                 = 299,
		F11                 = 300,
		F12                 = 301,
		F13                 = 302,
		F14                 = 303,
		F15                 = 304,
		F16                 = 305,
		F17                 = 306,
		F18                 = 307,
		F19                 = 308,
		F20                 = 309,
		F21                 = 310,
		F22                 = 311,
		F23                 = 312,
		F24                 = 313,
		F25                 = 314,

		/* Keypad */
		KP0                 = 320,
		KP1                 = 321,
		KP2                 = 322,
		KP3                 = 323,
		KP4                 = 324,
		KP5                 = 325,
		KP6                 = 326,
		KP7                 = 327,
		KP8                 = 328,
		KP9                 = 329,
		KPDecimal           = 330,
		KPDivide            = 331,
		KPMultiply          = 332,
		KPSubtract          = 333,
		KPAdd               = 334,
		KPEnter             = 335,
		KPEqual             = 336,

		LeftShift           = 340,
		LeftControl         = 341,
		LeftAlt             = 342,
		LeftSuper           = 343,
		RightShift          = 344,
		RightControl        = 345,
		RightAlt            = 346,
		RightSuper          = 347,
		Menu                = 348
	} Key;

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}
}

// From glfw3.h
#define AS_KEY_SPACE           ::as::Key::Space
#define AS_KEY_APOSTROPHE      ::as::Key::Apostrophe    /* ' */
#define AS_KEY_COMMA           ::as::Key::Comma         /* , */
#define AS_KEY_MINUS           ::as::Key::Minus         /* - */
#define AS_KEY_PERIOD          ::as::Key::Period        /* . */
#define AS_KEY_SLASH           ::as::Key::Slash         /* / */
#define AS_KEY_0               ::as::Key::D0
#define AS_KEY_1               ::as::Key::D1
#define AS_KEY_2               ::as::Key::D2
#define AS_KEY_3               ::as::Key::D3
#define AS_KEY_4               ::as::Key::D4
#define AS_KEY_5               ::as::Key::D5
#define AS_KEY_6               ::as::Key::D6
#define AS_KEY_7               ::as::Key::D7
#define AS_KEY_8               ::as::Key::D8
#define AS_KEY_9               ::as::Key::D9
#define AS_KEY_SEMICOLON       ::as::Key::Semicolon     /* ; */
#define AS_KEY_EQUAL           ::as::Key::Equal         /* = */
#define AS_KEY_A               ::as::Key::A
#define AS_KEY_B               ::as::Key::B
#define AS_KEY_C               ::as::Key::C
#define AS_KEY_D               ::as::Key::D
#define AS_KEY_E               ::as::Key::E
#define AS_KEY_F               ::as::Key::F
#define AS_KEY_G               ::as::Key::G
#define AS_KEY_H               ::as::Key::H
#define AS_KEY_I               ::as::Key::I
#define AS_KEY_J               ::as::Key::J
#define AS_KEY_K               ::as::Key::K
#define AS_KEY_L               ::as::Key::L
#define AS_KEY_M               ::as::Key::M
#define AS_KEY_N               ::as::Key::N
#define AS_KEY_O               ::as::Key::O
#define AS_KEY_P               ::as::Key::P
#define AS_KEY_Q               ::as::Key::Q
#define AS_KEY_R               ::as::Key::R
#define AS_KEY_S               ::as::Key::S
#define AS_KEY_T               ::as::Key::T
#define AS_KEY_U               ::as::Key::U
#define AS_KEY_V               ::as::Key::V
#define AS_KEY_W               ::as::Key::W
#define AS_KEY_X               ::as::Key::X
#define AS_KEY_Y               ::as::Key::Y
#define AS_KEY_Z               ::as::Key::Z
#define AS_KEY_LEFT_BRACKET    ::as::Key::LeftBracket   /* [ */
#define AS_KEY_BACKSLASH       ::as::Key::Backslash     /* \ */
#define AS_KEY_RIGHT_BRACKET   ::as::Key::RightBracket  /* ] */
#define AS_KEY_GRAVE_ACCENT    ::as::Key::GraveAccent   /* ` */
#define AS_KEY_WORLD_1         ::as::Key::World1        /* non-US #1 */
#define AS_KEY_WORLD_2         ::as::Key::World2        /* non-US #2 */

/* Function keys */
#define AS_KEY_ESCAPE          ::as::Key::Escape
#define AS_KEY_ENTER           ::as::Key::Enter
#define AS_KEY_TAB             ::as::Key::Tab
#define AS_KEY_BACKSPACE       ::as::Key::Backspace
#define AS_KEY_INSERT          ::as::Key::Insert
#define AS_KEY_DELETE          ::as::Key::Delete
#define AS_KEY_RIGHT           ::as::Key::Right
#define AS_KEY_LEFT            ::as::Key::Left
#define AS_KEY_DOWN            ::as::Key::Down
#define AS_KEY_UP              ::as::Key::Up
#define AS_KEY_PAGE_UP         ::as::Key::PageUp
#define AS_KEY_PAGE_DOWN       ::as::Key::PageDown
#define AS_KEY_HOME            ::as::Key::Home
#define AS_KEY_END             ::as::Key::End
#define AS_KEY_CAPS_LOCK       ::as::Key::CapsLock
#define AS_KEY_SCROLL_LOCK     ::as::Key::ScrollLock
#define AS_KEY_NUM_LOCK        ::as::Key::NumLock
#define AS_KEY_PRINT_SCREEN    ::as::Key::PrintScreen
#define AS_KEY_PAUSE           ::as::Key::Pause
#define AS_KEY_F1              ::as::Key::F1
#define AS_KEY_F2              ::as::Key::F2
#define AS_KEY_F3              ::as::Key::F3
#define AS_KEY_F4              ::as::Key::F4
#define AS_KEY_F5              ::as::Key::F5
#define AS_KEY_F6              ::as::Key::F6
#define AS_KEY_F7              ::as::Key::F7
#define AS_KEY_F8              ::as::Key::F8
#define AS_KEY_F9              ::as::Key::F9
#define AS_KEY_F10             ::as::Key::F10
#define AS_KEY_F11             ::as::Key::F11
#define AS_KEY_F12             ::as::Key::F12
#define AS_KEY_F13             ::as::Key::F13
#define AS_KEY_F14             ::as::Key::F14
#define AS_KEY_F15             ::as::Key::F15
#define AS_KEY_F16             ::as::Key::F16
#define AS_KEY_F17             ::as::Key::F17
#define AS_KEY_F18             ::as::Key::F18
#define AS_KEY_F19             ::as::Key::F19
#define AS_KEY_F20             ::as::Key::F20
#define AS_KEY_F21             ::as::Key::F21
#define AS_KEY_F22             ::as::Key::F22
#define AS_KEY_F23             ::as::Key::F23
#define AS_KEY_F24             ::as::Key::F24
#define AS_KEY_F25             ::as::Key::F25

/* Keypad */
#define AS_KEY_KP_0            ::as::Key::KP0
#define AS_KEY_KP_1            ::as::Key::KP1
#define AS_KEY_KP_2            ::as::Key::KP2
#define AS_KEY_KP_3            ::as::Key::KP3
#define AS_KEY_KP_4            ::as::Key::KP4
#define AS_KEY_KP_5            ::as::Key::KP5
#define AS_KEY_KP_6            ::as::Key::KP6
#define AS_KEY_KP_7            ::as::Key::KP7
#define AS_KEY_KP_8            ::as::Key::KP8
#define AS_KEY_KP_9            ::as::Key::KP9
#define AS_KEY_KP_DECIMAL      ::as::Key::KPDecimal
#define AS_KEY_KP_DIVIDE       ::as::Key::KPDivide
#define AS_KEY_KP_MULTIPLY     ::as::Key::KPMultiply
#define AS_KEY_KP_SUBTRACT     ::as::Key::KPSubtract
#define AS_KEY_KP_ADD          ::as::Key::KPAdd
#define AS_KEY_KP_ENTER        ::as::Key::KPEnter
#define AS_KEY_KP_EQUAL        ::as::Key::KPEqual

#define AS_KEY_LEFT_SHIFT      ::as::Key::LeftShift
#define AS_KEY_LEFT_CONTROL    ::as::Key::LeftControl
#define AS_KEY_LEFT_ALT        ::as::Key::LeftAlt
#define AS_KEY_LEFT_SUPER      ::as::Key::LeftSuper
#define AS_KEY_RIGHT_SHIFT     ::as::Key::RightShift
#define AS_KEY_RIGHT_CONTROL   ::as::Key::RightControl
#define AS_KEY_RIGHT_ALT       ::as::Key::RightAlt
#define AS_KEY_RIGHT_SUPER     ::as::Key::RightSuper
#define AS_KEY_MENU            ::as::Key::Menu
