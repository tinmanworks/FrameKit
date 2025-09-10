// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Input/KeyCodes.h
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Keyboard key codes for FrameKit. Values match GLFW key codes.
// =============================================================================

#pragma once

#include <cstdint>

namespace FrameKit {

	// Strongly typed key code
	enum class KeyCode : std::uint16_t {};

	namespace Key {

		// From glfw3.h (values preserved)
		constexpr KeyCode Space = static_cast<KeyCode>(32);
		constexpr KeyCode Apostrophe = static_cast<KeyCode>(39); // '
		constexpr KeyCode Comma = static_cast<KeyCode>(44); // ,
		constexpr KeyCode Minus = static_cast<KeyCode>(45); // -
		constexpr KeyCode Period = static_cast<KeyCode>(46); // .
		constexpr KeyCode Slash = static_cast<KeyCode>(47); // /

		constexpr KeyCode D0 = static_cast<KeyCode>(48);
		constexpr KeyCode D1 = static_cast<KeyCode>(49);
		constexpr KeyCode D2 = static_cast<KeyCode>(50);
		constexpr KeyCode D3 = static_cast<KeyCode>(51);
		constexpr KeyCode D4 = static_cast<KeyCode>(52);
		constexpr KeyCode D5 = static_cast<KeyCode>(53);
		constexpr KeyCode D6 = static_cast<KeyCode>(54);
		constexpr KeyCode D7 = static_cast<KeyCode>(55);
		constexpr KeyCode D8 = static_cast<KeyCode>(56);
		constexpr KeyCode D9 = static_cast<KeyCode>(57);

		constexpr KeyCode Semicolon = static_cast<KeyCode>(59); // ;
		constexpr KeyCode Equal = static_cast<KeyCode>(61); // =

		constexpr KeyCode A = static_cast<KeyCode>(65);
		constexpr KeyCode B = static_cast<KeyCode>(66);
		constexpr KeyCode C = static_cast<KeyCode>(67);
		constexpr KeyCode D = static_cast<KeyCode>(68);
		constexpr KeyCode E = static_cast<KeyCode>(69);
		constexpr KeyCode F = static_cast<KeyCode>(70);
		constexpr KeyCode G = static_cast<KeyCode>(71);
		constexpr KeyCode H = static_cast<KeyCode>(72);
		constexpr KeyCode I = static_cast<KeyCode>(73);
		constexpr KeyCode J = static_cast<KeyCode>(74);
		constexpr KeyCode K = static_cast<KeyCode>(75);
		constexpr KeyCode L = static_cast<KeyCode>(76);
		constexpr KeyCode M = static_cast<KeyCode>(77);
		constexpr KeyCode N = static_cast<KeyCode>(78);
		constexpr KeyCode O = static_cast<KeyCode>(79);
		constexpr KeyCode P = static_cast<KeyCode>(80);
		constexpr KeyCode Q = static_cast<KeyCode>(81);
		constexpr KeyCode R = static_cast<KeyCode>(82);
		constexpr KeyCode S = static_cast<KeyCode>(83);
		constexpr KeyCode T = static_cast<KeyCode>(84);
		constexpr KeyCode U = static_cast<KeyCode>(85);
		constexpr KeyCode V = static_cast<KeyCode>(86);
		constexpr KeyCode W = static_cast<KeyCode>(87);
		constexpr KeyCode X = static_cast<KeyCode>(88);
		constexpr KeyCode Y = static_cast<KeyCode>(89);
		constexpr KeyCode Z = static_cast<KeyCode>(90);

		constexpr KeyCode LeftBracket = static_cast<KeyCode>(91); // [
		constexpr KeyCode Backslash = static_cast<KeyCode>(92); // '\'
		constexpr KeyCode RightBracket = static_cast<KeyCode>(93); // ]
		constexpr KeyCode GraveAccent = static_cast<KeyCode>(96); // `

		constexpr KeyCode World1 = static_cast<KeyCode>(161); // non-US #1
		constexpr KeyCode World2 = static_cast<KeyCode>(162); // non-US #2

		// Function keys
		constexpr KeyCode Escape = static_cast<KeyCode>(256);
		constexpr KeyCode Enter = static_cast<KeyCode>(257);
		constexpr KeyCode Tab = static_cast<KeyCode>(258);
		constexpr KeyCode Backspace = static_cast<KeyCode>(259);
		constexpr KeyCode Insert = static_cast<KeyCode>(260);
		constexpr KeyCode Delete = static_cast<KeyCode>(261);
		constexpr KeyCode Right = static_cast<KeyCode>(262);
		constexpr KeyCode Left = static_cast<KeyCode>(263);
		constexpr KeyCode Down = static_cast<KeyCode>(264);
		constexpr KeyCode Up = static_cast<KeyCode>(265);
		constexpr KeyCode PageUp = static_cast<KeyCode>(266);
		constexpr KeyCode PageDown = static_cast<KeyCode>(267);
		constexpr KeyCode Home = static_cast<KeyCode>(268);
		constexpr KeyCode End = static_cast<KeyCode>(269);
		constexpr KeyCode CapsLock = static_cast<KeyCode>(280);
		constexpr KeyCode ScrollLock = static_cast<KeyCode>(281);
		constexpr KeyCode NumLock = static_cast<KeyCode>(282);
		constexpr KeyCode PrintScreen = static_cast<KeyCode>(283);
		constexpr KeyCode Pause = static_cast<KeyCode>(284);
		constexpr KeyCode F1 = static_cast<KeyCode>(290);
		constexpr KeyCode F2 = static_cast<KeyCode>(291);
		constexpr KeyCode F3 = static_cast<KeyCode>(292);
		constexpr KeyCode F4 = static_cast<KeyCode>(293);
		constexpr KeyCode F5 = static_cast<KeyCode>(294);
		constexpr KeyCode F6 = static_cast<KeyCode>(295);
		constexpr KeyCode F7 = static_cast<KeyCode>(296);
		constexpr KeyCode F8 = static_cast<KeyCode>(297);
		constexpr KeyCode F9 = static_cast<KeyCode>(298);
		constexpr KeyCode F10 = static_cast<KeyCode>(299);
		constexpr KeyCode F11 = static_cast<KeyCode>(300);
		constexpr KeyCode F12 = static_cast<KeyCode>(301);
		constexpr KeyCode F13 = static_cast<KeyCode>(302);
		constexpr KeyCode F14 = static_cast<KeyCode>(303);
		constexpr KeyCode F15 = static_cast<KeyCode>(304);
		constexpr KeyCode F16 = static_cast<KeyCode>(305);
		constexpr KeyCode F17 = static_cast<KeyCode>(306);
		constexpr KeyCode F18 = static_cast<KeyCode>(307);
		constexpr KeyCode F19 = static_cast<KeyCode>(308);
		constexpr KeyCode F20 = static_cast<KeyCode>(309);
		constexpr KeyCode F21 = static_cast<KeyCode>(310);
		constexpr KeyCode F22 = static_cast<KeyCode>(311);
		constexpr KeyCode F23 = static_cast<KeyCode>(312);
		constexpr KeyCode F24 = static_cast<KeyCode>(313);
		constexpr KeyCode F25 = static_cast<KeyCode>(314);

		// Keypad
		constexpr KeyCode KP0 = static_cast<KeyCode>(320);
		constexpr KeyCode KP1 = static_cast<KeyCode>(321);
		constexpr KeyCode KP2 = static_cast<KeyCode>(322);
		constexpr KeyCode KP3 = static_cast<KeyCode>(323);
		constexpr KeyCode KP4 = static_cast<KeyCode>(324);
		constexpr KeyCode KP5 = static_cast<KeyCode>(325);
		constexpr KeyCode KP6 = static_cast<KeyCode>(326);
		constexpr KeyCode KP7 = static_cast<KeyCode>(327);
		constexpr KeyCode KP8 = static_cast<KeyCode>(328);
		constexpr KeyCode KP9 = static_cast<KeyCode>(329);
		constexpr KeyCode KPDecimal = static_cast<KeyCode>(330);
		constexpr KeyCode KPDivide = static_cast<KeyCode>(331);
		constexpr KeyCode KPMultiply = static_cast<KeyCode>(332);
		constexpr KeyCode KPSubtract = static_cast<KeyCode>(333);
		constexpr KeyCode KPAdd = static_cast<KeyCode>(334);
		constexpr KeyCode KPEnter = static_cast<KeyCode>(335);
		constexpr KeyCode KPEqual = static_cast<KeyCode>(336);

		constexpr KeyCode LeftShift = static_cast<KeyCode>(340);
		constexpr KeyCode LeftControl = static_cast<KeyCode>(341);
		constexpr KeyCode LeftAlt = static_cast<KeyCode>(342);
		constexpr KeyCode LeftSuper = static_cast<KeyCode>(343);
		constexpr KeyCode RightShift = static_cast<KeyCode>(344);
		constexpr KeyCode RightControl = static_cast<KeyCode>(345);
		constexpr KeyCode RightAlt = static_cast<KeyCode>(346);
		constexpr KeyCode RightSuper = static_cast<KeyCode>(347);
		constexpr KeyCode Menu = static_cast<KeyCode>(348);

	} // namespace Key
} // namespace FrameKit
