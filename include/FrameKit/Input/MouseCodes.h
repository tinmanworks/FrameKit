// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Input/MouseCodes.h
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Mouse button codes for FrameKit. Values match GLFW button codes.
// =============================================================================

#pragma once

#include <cstdint>

namespace FrameKit {

	// Strongly typed mouse code
	enum class MouseCode : std::uint16_t {};

	namespace Mouse {

		// From glfw3.h (values preserved)
		constexpr MouseCode Button0 = static_cast<MouseCode>(0);
		constexpr MouseCode Button1 = static_cast<MouseCode>(1);
		constexpr MouseCode Button2 = static_cast<MouseCode>(2);
		constexpr MouseCode Button3 = static_cast<MouseCode>(3);
		constexpr MouseCode Button4 = static_cast<MouseCode>(4);
		constexpr MouseCode Button5 = static_cast<MouseCode>(5);
		constexpr MouseCode Button6 = static_cast<MouseCode>(6);
		constexpr MouseCode Button7 = static_cast<MouseCode>(7);

		constexpr MouseCode ButtonLast = Button7;
		constexpr MouseCode ButtonLeft = Button0;
		constexpr MouseCode ButtonRight = Button1;
		constexpr MouseCode ButtonMiddle = Button2;

	} // namespace Mouse
} // namespace FrameKit
