#pragma once

#include "renderer/imgui/imgui_impl_sdl_gl3.hpp"
#include <SDL.h>
#include <iostream>
#include "glm/glm.hpp"
#include "shared/Matrices.h"
#include <openvr.h>

class Input{
private:
	//keyboard
    static bool _last_kb[];
    static bool _now_kb[];

	//mouse
    static glm::vec2 _last_mouse;
    static glm::vec2 _current_mouse;

	//VR
	static vr::VRControllerState001_t _vr_controller[];

public:


    bool quit = false;

    void update();


	//keyboard
    static bool get_key_down(Uint8 key);
    static bool get_key_on_down(Uint8 key);

	//mouse
    static glm::vec2 get_mouse_delta();
    static glm::vec2 get_mouse_position();


	//VR

	void update_vr_pose();

	enum VRDevice {
		Invalid,
		HMD,
		Controller,
		GenericTracker,
		TrackingReference,
		DisplayRedirect,
	};

	static vr::TrackedDevicePose_t device_pose[vr::k_unMaxTrackedDeviceCount];
	static VRDevice device_type[vr::k_unMaxTrackedDeviceCount];
	static glm::mat4 device_matrix[vr::k_unMaxTrackedDeviceCount];

	static int num_controllers;

	vr::IVRSystem* vr_hmd;

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

		Max = 64,
	};

	static bool get_vr_button_down(int controller_index, VRButton button);
	static bool get_vr_button_touch(int controller_index, VRButton button);

	static glm::vec2 get_vr_axis(int controller_index, int axis_index);


	static glm::mat4 get_vr_controller_matrix(int controller_index);
	static glm::mat4 get_hmd_matrix();

private:

	static int _hmd_index;
	static int _controller_indices[];

	glm::mat4 convert_HmdMatrix_to_mat4(vr::HmdMatrix34_t &matPose);

};
