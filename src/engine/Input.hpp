#pragma once

#include "renderer/imgui/imgui_impl_sdl_gl3.hpp"
#include <SDL.h>
#include <iostream>
#include "glm/glm.hpp"
#include "shared/Matrices.h"

namespace vr {
	class IVRSystem;
	struct VRControllerState001_t;
};


class Input{
private:
    static bool _last_kb[];
    static bool _now_kb[];

    static glm::vec2 _last_mouse;
    static glm::vec2 _current_mouse;

	//vr
	static vr::VRControllerState001_t _vr_controller[];

public:

	static glm::mat4 head_matrix;
	static glm::mat4 controller_matrix[2];

	vr::IVRSystem* vr_hmd;

    bool quit = false;

    void update();

    static bool get_key_down(Uint8 key);
    static bool get_key_on_down(Uint8 key);

    static glm::vec2 get_mouse_delta();
    static glm::vec2 get_mouse_position();


	enum VRButton
	{
		System = 0,
		ApplicationMenu = 1,
		Grip = 2,
		DPad_Left = 3,
		DPad_Up = 4,
		DPad_Right = 5,
		DPad_Down = 6,
		A = 7,

		ProximitySensor = 31,

		Axis0 = 32,
		Axis1 = 33,
		Axis2 = 34,
		Axis3 = 35,
		Axis4 = 36,

		// aliases for well known controllers
		SteamVR_Touchpad = Axis0,
		SteamVR_Trigger = Axis1,

		Dashboard_Back = Grip,

		Max = 64
	};

	static bool get_vr_button_down(int controller_index, VRButton button);
	static bool get_vr_button_touch(int controller_index, VRButton button);

	static glm::vec2 get_vr_axis(int controller_index, int axis_index);


};
