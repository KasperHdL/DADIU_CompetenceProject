#include <engine/Input.hpp>
#include <openvr.h>

bool Input::_last_kb[284] = {0};
bool Input::_now_kb[284] = {0};

vr::VRControllerState001_t Input::_vr_controller[3] = { 0 };

glm::mat4 Input::head_matrix = glm::mat4();
glm::mat4 Input::controller_matrix[2] = { glm::mat4(), glm::mat4() };

glm::vec2 Input::_last_mouse = glm::vec2();
glm::vec2 Input::_current_mouse = glm::vec2();

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
        }
    }

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

bool Input::get_key_down(Uint8 key){
    return Input::_now_kb[key];
}

bool Input::get_key_on_down(Uint8 key){
    return (Input::_last_kb[key] ^ Input::_now_kb[key]) & Input::_now_kb[key];
}

glm::vec2 Input::get_mouse_delta(){
    return _current_mouse - _last_mouse;
}

glm::vec2 Input::get_mouse_position(){
    return _current_mouse;
}

bool Input::get_vr_button_down(int controller_index, VRButton button) {
	uint64_t mask = vr::ButtonMaskFromId((vr::EVRButtonId)button);
	uint64_t val = _vr_controller[controller_index].ulButtonPressed;
	uint64_t r = (mask & val) != 0;

	std::cout << controller_index << ": mask: " << mask << "\tval: " << val << "\tresult: " << r << "\n";

	return r;
}

bool Input::get_vr_button_touch(int controller_index, VRButton button) {
	uint64_t mask = vr::ButtonMaskFromId((vr::EVRButtonId)button);
	uint64_t val = _vr_controller[controller_index].ulButtonTouched;
	uint64_t r = (mask & val) != 0;

	std::cout << controller_index << ": mask: " << mask << "\nval: " << val << "\nresult: " << r << "\n";

	return r;
}

glm::vec2 Input::get_vr_axis(int controller_index, int axis_index) {

	glm::vec2 vector;

	if (axis_index < 0 || axis_index > vr::k_unControllerStateAxisCount)
		return vector;
		
	vr::VRControllerAxis_t axis = _vr_controller[controller_index].rAxis[axis_index];

	vector.x = axis.x;
	vector.y = axis.y;

	return vector;
}
