#include <engine/Input.hpp>
#include <string>
#include <iostream>

//keyboard
bool Input::_last_kb[284] = {0};
bool Input::_now_kb[284] = {0};

//mouse
glm::vec2 Input::_last_mouse = glm::vec2();
glm::vec2 Input::_current_mouse = glm::vec2();

//vr
vr::VRControllerState001_t Input::_vr_controller[vr::k_unMaxTrackedDeviceCount] = { 0 };

vr::TrackedDevicePose_t Input::device_pose[vr::k_unMaxTrackedDeviceCount];
Input::VRDevice Input::device_type[vr::k_unMaxTrackedDeviceCount] = { Input::VRDevice::Invalid };
glm::mat4 Input::device_matrix[vr::k_unMaxTrackedDeviceCount] = { glm::mat4() };

int Input::_hmd_index = -1;
int Input::_controller_indices[vr::k_unMaxTrackedDeviceCount] = { -1 };
int Input::num_controllers = 0;

glm::vec2 Input::vr_playarea = glm::vec2();

void Input::update(){
    for(int i = 0; i < 284; i++)
        _last_kb[i] = _now_kb[i];

    _last_mouse = _current_mouse;

    SDL_Event e; //An SDL event to figure out which keys are being manipulated
    while (SDL_PollEvent(&e) != 0) { //If there is an event

        ImGui_ImplSdlGL3_ProcessEvent(&e);

        switch (e.type){
            case SDL_KEYDOWN:
                _now_kb[e.key.keysym.scancode] = true;
            break;
            case SDL_KEYUP:
                _now_kb[e.key.keysym.scancode] = false;
            break;
            case SDL_QUIT:
                quit = true;
            break;
            case SDL_MOUSEMOTION:
				//@TODO(KASPER) e.motion.x is a Sin32
                _current_mouse.x = (float)e.motion.x;
                _current_mouse.y = (float)e.motion.y;
            break;
			//mouse button event
        }
    }


	//VR
	if (!vr_hmd)
		return;

	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		if (!vr_hmd)
			continue;

		vr::VRControllerState_t state;

		if (vr_hmd->GetControllerState(unDevice, &state, sizeof(state)))
		{
			_vr_controller[unDevice] = state;
		}

	}

}

void Input::update_vr_pose() {
	if (!vr_hmd)
		return;

	vr::VRCompositor()->WaitGetPoses(device_pose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	std::string debug = "";

	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) {
		if (device_pose[i].bPoseIsValid) {
			device_matrix[i] = convert_HmdMatrix_to_mat4(device_pose[i].mDeviceToAbsoluteTracking);

			device_type[i] = (VRDevice)vr_hmd->GetTrackedDeviceClass(i);
			int controller_count = 0;

			if (device_type[i] == VRDevice::HMD) {
				device_matrix[i] = inverse(device_matrix[i]);
				_hmd_index = i;
			}else if (device_type[i] == VRDevice::Controller) {
				_controller_indices[controller_count++] = i;
			}

			num_controllers = controller_count;

			char c;
			switch ((VRDevice)vr_hmd->GetTrackedDeviceClass(i)) {
				case VRDevice::Controller: c = 'C'; break;
				case VRDevice::HMD: c = 'H'; break;
				case VRDevice::Invalid: c = 'I'; break;
				case VRDevice::GenericTracker: c = 'G'; break;
				case VRDevice::TrackingReference: c = 'T'; break;
				default: c = '?'; break;
			}

			debug += c;
		}
	}

//	std::cout << debug << "\n";
}

void Input::set_chaperone(vr::IVRChaperone* chaperone) {
	vr_chaperone = chaperone;
	vr_chaperone->GetPlayAreaSize(&vr_playarea.x, &vr_playarea.y);
	std::cout << "playarea = [" << vr_playarea.x << ", " << vr_playarea.y << "]\n";
}



//Keyboard

bool Input::get_key_down(Uint8 key){
    return Input::_now_kb[key];
}

bool Input::get_key_on_down(Uint8 key){
    return (Input::_last_kb[key] ^ Input::_now_kb[key]) & Input::_now_kb[key];
}

//Mouse
glm::vec2 Input::get_mouse_delta(){
    return _current_mouse - _last_mouse;
}

glm::vec2 Input::get_mouse_position(){
    return _current_mouse;
}

//VR
bool Input::get_vr_button_down(int controller_index, VRButton button) {
	if (controller_index >= num_controllers) return false;
	int index = _controller_indices[controller_index];

	uint64_t mask = vr::ButtonMaskFromId((vr::EVRButtonId)button);
	uint64_t val = _vr_controller[index].ulButtonPressed;
	
	return (mask & val) != 0;
}

bool Input::get_vr_button_touch(int controller_index, VRButton button) {
	if (controller_index >= num_controllers) return false;
	int index = _controller_indices[controller_index];

	uint64_t mask = vr::ButtonMaskFromId((vr::EVRButtonId)button);
	uint64_t val = _vr_controller[index].ulButtonTouched;

	return (mask & val) != 0;
}

glm::vec2 Input::get_vr_axis(int controller_index, int axis_index) {
	if (controller_index >= num_controllers) return glm::vec2();

	glm::vec2 vector;

	if (axis_index < 0 || axis_index > vr::k_unControllerStateAxisCount)
		return vector;
	
	int index = _controller_indices[controller_index];
	vr::VRControllerAxis_t axis = _vr_controller[index].rAxis[axis_index];

	vector.x = axis.x;
	vector.y = axis.y;

	return vector;
}

glm::mat4 Input::get_vr_controller_matrix(int controller_index) {
	if (controller_index >= num_controllers) return glm::mat4();
	int index = _controller_indices[controller_index];
	return device_matrix[index];
}

glm::mat4 Input::get_hmd_matrix() {
	return device_matrix[_hmd_index];
}


//Helper functions
glm::mat4 Input::convert_HmdMatrix_to_mat4(vr::HmdMatrix34_t &matPose) {
	glm::mat4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	return matrixObj;
}


