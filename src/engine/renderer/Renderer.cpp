#include <engine/renderer/Renderer.hpp>
#include <engine/Engine.hpp>
#include <debug/DebugInterface.hpp>
#include <engine/renderer/Camera.hpp>
#include <engine/Entity.hpp>


Renderer* Renderer::instance = nullptr;

Renderer::Renderer(){}

Renderer::~Renderer(){
    SDL_GL_DeleteContext(glcontext);
    instance = nullptr;
}

bool Renderer::initialize(SDL_Window* window, int screen_width, int screen_height){
    if (instance != nullptr){
        std::cerr << "Multiple versions of Renderer initialized. Only a single instance is supported." << std::endl;
    }

	instance = this;
	

	memset(m_rDevClassChar, 0, sizeof(m_rDevClassChar));

	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);

	if (eError != vr::VRInitError_None)
	{
		m_pHMD = NULL;
		char buf[1024];
		sprintf_s(buf, sizeof(buf), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL);
		return false;
	}


	m_pRenderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
	if (!m_pRenderModels)
	{
		m_pHMD = NULL;
		vr::VR_Shutdown();

		char buf[1024];
		sprintf_s(buf, sizeof(buf), "Unable to get render model interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL);
		return false;
	}


    this->window        = window;
    this->screen_width  = screen_width;
    this->screen_height = screen_height;

    glcontext = SDL_GL_CreateContext(window);

    glewExperimental = GL_FALSE;
    GLenum err = glewInit();


    if(err != GLEW_OK)
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);


	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);



    //camera
    camera = new Camera();
    camera->set_viewport(0,0,screen_width, screen_height);
    camera->set_perspective_projection();

    ambient_light = vec4(0.2f);



	// cube array
	m_iSceneVolumeWidth = 20;
	m_iSceneVolumeHeight = 20;
	m_iSceneVolumeDepth = 20;


	CreateAllShaders();
	
	SetupScene();
	SetupCameras();
	SetupStereoRenderTargets();
	SetupCompanionWindow();
	SetupRenderModels();
	
	vr::EVRInitError peError = vr::VRInitError_None;

	if ( !vr::VRCompositor() )
	{
		printf( "Compositor initialization failed. See log file for details\n" );
		return false;
	}

		

	return true;
}


void Renderer::render(float delta_time){


    glDepthMask(GL_TRUE);
    glClearColor(ambient_light.r, ambient_light.g, ambient_light.b, ambient_light.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    int w,h;
    SDL_GetWindowSize(window,&w,&h);

    time += delta_time;

    camera->set_viewport(0,0,w,h);
    camera->set_perspective_projection(); 


	_render_scene();

    debug->render(delta_time);

    SDL_GL_SwapWindow(window);

    return;
}


/////////////////
//RENDER METHODS
/////////////////

void Renderer::_render_entity(Entity* entity){

    if(entity->transform == nullptr){
        cout << entity->name << "->transform = nullptr\n";

        return;
    }

	test_shader->set_uniform("matrix", current_projection_matrix * current_view_matrix * current_model_matrix * entity->transform->get_model_transform());
	test_shader->set_uniform("color", entity->color);
	/*
    //vert
    scene_shader->set_uniform("model"      , current_model_matrix);
	scene_shader->set_uniform("normalMat"  , entity->transform->get_normal_transform());

	scene_shader->set_uniform("view"       , current_view_matrix);
	scene_shader->set_uniform("projection" , current_projection_matrix);

	scene_shader->set_uniform("ambientLight", ambient_light);
	scene_shader->set_uniform("color", entity->color);

    //lights
    for(int i = 0; i < 4;i++){
        Light* l = God::lights[i];
        if(l != nullptr){

            vec4 light_pos_type = vec4(l->position, (int)l->type); 
            vec4 light_color_range = vec4(l->color * l->intensity, l->range);

			scene_shader->set_uniform("lightPosType[" + to_string(i) + "]", light_pos_type);
			scene_shader->set_uniform("lightColorRange[" + to_string(i) + "]", light_color_range);

        }
    }
	*/

    int indexCount = (int) entity->mesh->indices.size();
    if (indexCount == 0){
        glDrawArrays((GLenum)entity->mesh->topology, 0, entity->mesh->vertex_count);
    } else {
        glDrawElements((GLenum) entity->mesh->topology, indexCount, GL_UNSIGNED_SHORT, 0);
    }

}

void Renderer::_render_pool(DynamicPool<Entity*> pool){
    for(int i = 0; i < pool.capacity;i++){
        Entity* e = *pool[i];
        if(e != nullptr){
            cout << "i " << i << "\n";
            _render_entity(e);
        }
    }
}

void Renderer::_render_scene() {

	test_shader->use();

	//CUBE
	if (God::cube_mesh_entities.count > 0) {
		Mesh::get_cube()->bind();

		for (int i = 0; i < God::cube_mesh_entities.capacity; i++) {
			Entity** p = God::cube_mesh_entities[i];
			if (p != nullptr) {
				Entity* e = *p;
				if (e->mesh != nullptr) {
					_render_entity(e);
				}
			}
		}
	}

	//SPHERE
	if (God::sphere_mesh_entities.count > 0) {
		Mesh::get_sphere()->bind();

		for (int i = 0; i < God::sphere_mesh_entities.capacity; i++) {
			Entity** p = God::sphere_mesh_entities[i];
			if (p != nullptr) {
				Entity* e = *p;
				if (e->mesh != nullptr) {
					_render_entity(e);
				}
			}
		}
	}

	//QUADS
	if (God::quad_mesh_entities.count > 0) {
		Mesh::get_quad()->bind();

		for (int i = 0; i < God::quad_mesh_entities.capacity; i++) {
			Entity** p = God::quad_mesh_entities[i];
			if (p != nullptr) {
				Entity* e = *p;
				if (e->mesh != nullptr) {
					_render_entity(e);
				}
			}
		}
	}

	//CUSTOM
	if (God::custom_mesh_entities.count > 0) {
		for (int i = 0; i < God::custom_mesh_entities.capacity; i++) {
			Entity** p = God::custom_mesh_entities[i];
			if (p != nullptr) {
				Entity* e = *p;
				if (e->mesh != nullptr) {
					e->mesh->bind();
					_render_entity(e);
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------
void Renderer::ProcessVREvent(const vr::VREvent_t & event)
{
	switch (event.eventType)
	{
	case vr::VREvent_TrackedDeviceActivated:
	{
		SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
		printf("Device %u attached. Setting up render model.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceDeactivated:
	{
		printf("Device %u detached.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceUpdated:
	{
		printf("Device %u updated.\n", event.trackedDeviceIndex);
	}
	break;
	}
}

void Renderer::RenderFrame(float delta_time)
{

	// Process SteamVR events
	vr::VREvent_t event;
	while (m_pHMD->PollNextEvent(&event, sizeof(event)))
	{
		ProcessVREvent(event);
	}

	// Process SteamVR controller state
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		vr::VRControllerState_t state;
		if (m_pHMD->GetControllerState(unDevice, &state, sizeof(state)))
		{
			m_rbShowTrackedDevice[unDevice] = state.ulButtonPressed == 0;
		}
	}


	if (m_pHMD)
	{
		RenderControllerAxes();
		_render_stereo_targets();
		RenderCompanionWindow();

		vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)left_eye_description.resolve_texture_id, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)right_eye_description.resolve_texture_id, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
	}


	debug->render(delta_time);


	SDL_GL_SwapWindow(window);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	UpdateHMDMatrixPose();
}


void Renderer::RenderCompanionWindow()
{
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, screen_width, screen_height);

	glBindVertexArray(m_unCompanionWindowVAO);
	glUseProgram(m_unCompanionWindowProgramID);

	// render left eye (first half of index array )
	glBindTexture(GL_TEXTURE_2D, left_eye_description.resolve_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDrawElements(GL_TRIANGLES, m_uiCompanionWindowIndexSize / 2, GL_UNSIGNED_SHORT, 0);

	// render right eye (second half of index array )
	glBindTexture(GL_TEXTURE_2D, right_eye_description.resolve_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDrawElements(GL_TRIANGLES, m_uiCompanionWindowIndexSize / 2, GL_UNSIGNED_SHORT, (const void *)(uintptr_t)(m_uiCompanionWindowIndexSize));

	glBindVertexArray(0);
	glUseProgram(0);
}

void Renderer::_render_stereo_targets()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_MULTISAMPLE);

	// Left Eye
	glBindFramebuffer(GL_FRAMEBUFFER, left_eye_description.framebuffer_id);
	glViewport(0, 0, vr_screen_width, vr_screen_height);
	RenderScene(vr::Eye_Left);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, left_eye_description.framebuffer_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, left_eye_description.resolve_framebuffer_id);

	glBlitFramebuffer(0, 0, vr_screen_width, vr_screen_height, 0, 0, vr_screen_width, vr_screen_height,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glEnable(GL_MULTISAMPLE);


	// Right Eye
	glBindFramebuffer(GL_FRAMEBUFFER, right_eye_description.framebuffer_id);
	glViewport(0, 0, vr_screen_width, vr_screen_height);
	RenderScene(vr::Eye_Right);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, right_eye_description.framebuffer_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, right_eye_description.resolve_framebuffer_id);

	glBlitFramebuffer(0, 0, vr_screen_width, vr_screen_height, 0, 0, vr_screen_width, vr_screen_height,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); 
}


void Renderer::RenderScene(vr::Hmd_Eye nEye) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	current_model_matrix = m_mat4HMDPose;

	if (nEye == vr::Eye_Left)
	{
		current_view_matrix = m_mat4eyePosLeft;
		current_projection_matrix = m_mat4ProjectionLeft;
	}
	else if (nEye == vr::Eye_Right)
	{
		current_view_matrix = m_mat4eyePosRight;
		current_projection_matrix = m_mat4ProjectionRight;
	}


	glUseProgram(m_unSceneProgramID);
	glUniformMatrix4fv(m_nSceneMatrixLocation, 1, GL_FALSE, glm::value_ptr(GetCurrentViewProjectionMatrix(nEye)));
	glBindVertexArray(m_unSceneVAO);
	glBindTexture(GL_TEXTURE_2D, m_iTexture);
	glDrawArrays(GL_TRIANGLES, 0, m_uiVertcount);
	glBindVertexArray(0);


	// draw the controller axis lines
	glUseProgram(m_unControllerTransformProgramID);
	glUniformMatrix4fv(m_nControllerMatrixLocation, 1, GL_FALSE, glm::value_ptr(GetCurrentViewProjectionMatrix(nEye)));
	glBindVertexArray(m_unControllerVAO);
	glDrawArrays(GL_LINES, 0, m_uiControllerVertcount);
	glBindVertexArray(0);

	// ----- Render Model rendering -----
	glUseProgram(m_unRenderModelProgramID);

	for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_rTrackedDeviceToRenderModel[unTrackedDevice] || !m_rbShowTrackedDevice[unTrackedDevice])
			continue;

		const vr::TrackedDevicePose_t & pose = m_rTrackedDevicePose[unTrackedDevice];
		if (!pose.bPoseIsValid)
			continue;

		mat4 matMVP = GetCurrentViewProjectionMatrix(nEye) * m_rmat4DevicePose[unTrackedDevice];
		glUniformMatrix4fv(m_nRenderModelMatrixLocation, 1, GL_FALSE, glm::value_ptr(matMVP));

		m_rTrackedDeviceToRenderModel[unTrackedDevice]->Draw();
	}

}

///////////
// UTILS
///////////

bool Renderer::_create_framebuffer(int width, int height, Renderer::FrameBufferDescription &framebuffer)
{
	glGenFramebuffers(1, &framebuffer.framebuffer_id);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.framebuffer_id);

	glGenRenderbuffers(1, &framebuffer.depthbuffer_id);
	glBindRenderbuffer(GL_RENDERBUFFER, framebuffer.depthbuffer_id);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer.depthbuffer_id);

	glGenTextures(1, &framebuffer.rendertexture_id);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebuffer.rendertexture_id);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, width, height, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebuffer.rendertexture_id, 0);

	glGenFramebuffers(1, &framebuffer.resolve_framebuffer_id);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.resolve_framebuffer_id);

	glGenTextures(1, &framebuffer.resolve_texture_id);
	glBindTexture(GL_TEXTURE_2D, framebuffer.resolve_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.resolve_texture_id, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void Renderer::SetupCameras()
{
	m_mat4ProjectionLeft = GetHMDMatrixProjectionEye(vr::Eye_Left);
	m_mat4ProjectionRight = GetHMDMatrixProjectionEye(vr::Eye_Right);
	m_mat4eyePosLeft = GetHMDMatrixPoseEye(vr::Eye_Left);
	m_mat4eyePosRight = GetHMDMatrixPoseEye(vr::Eye_Right);
}

void Renderer::SetupCompanionWindow()
{
	if (!m_pHMD)
		return;

	std::vector<VertexDataWindow> vVerts;

	// left eye verts
	vVerts.push_back(VertexDataWindow(vec2(-1, -1), vec2(0, 1)));
	vVerts.push_back(VertexDataWindow(vec2(0, -1), vec2(1, 1)));
	vVerts.push_back(VertexDataWindow(vec2(-1, 1), vec2(0, 0)));
	vVerts.push_back(VertexDataWindow(vec2(0, 1), vec2(1, 0)));

	// right eye verts
	vVerts.push_back(VertexDataWindow(vec2(0, -1), vec2(0, 1)));
	vVerts.push_back(VertexDataWindow(vec2(1, -1), vec2(1, 1)));
	vVerts.push_back(VertexDataWindow(vec2(0, 1), vec2(0, 0)));
	vVerts.push_back(VertexDataWindow(vec2(1, 1), vec2(1, 0)));

	GLushort vIndices[] = { 0, 1, 3,   0, 3, 2,   4, 5, 7,   4, 7, 6 };
	m_uiCompanionWindowIndexSize = _countof(vIndices);

	glGenVertexArrays(1, &m_unCompanionWindowVAO);
	glBindVertexArray(m_unCompanionWindowVAO);

	glGenBuffers(1, &m_glCompanionWindowIDVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_glCompanionWindowIDVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, vVerts.size() * sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_glCompanionWindowIDIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glCompanionWindowIDIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_uiCompanionWindowIndexSize * sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, texCoord));

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Renderer::SetupStereoRenderTargets()
{
	if (!m_pHMD)
		return false;

	m_pHMD->GetRecommendedRenderTargetSize(&vr_screen_width, &vr_screen_height);

	_create_framebuffer(vr_screen_width, vr_screen_height, left_eye_description);
	_create_framebuffer(vr_screen_width, vr_screen_height, right_eye_description);

	return true;
}





//-----------------------------------------------------------------------------
// Purpose: Gets a Matrix Projection Eye with respect to nEye.
//-----------------------------------------------------------------------------
mat4 Renderer::GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
{
	if (!m_pHMD)
		return mat4();

	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, vr_nearclip, vr_farclip);

	return mat4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}


//-----------------------------------------------------------------------------
// Purpose: Gets an HMDMatrixPoseEye with respect to nEye.
//-----------------------------------------------------------------------------
mat4 Renderer::GetHMDMatrixPoseEye(vr::Hmd_Eye nEye)
{
	if (!m_pHMD)
		return mat4();

	vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform(nEye);
	mat4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
	);

	return inverse(matrixObj);
}


//-----------------------------------------------------------------------------
// Purpose: Gets a Current View Projection Matrix with respect to nEye,
//          which may be an Eye_Left or an Eye_Right.
//-----------------------------------------------------------------------------
mat4 Renderer::GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye)
{
	mat4 matMVP;
	if (nEye == vr::Eye_Left)
	{
		matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
	}
	else if (nEye == vr::Eye_Right)
	{
		matMVP = m_mat4ProjectionRight * m_mat4eyePosRight *  m_mat4HMDPose;
	}

	return matMVP;
}


void Renderer::UpdateHMDMatrixPose()
{
	if (!m_pHMD)
		return;

	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	m_iValidPoseCount = 0;
	m_strPoseClasses = "";
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
		{
			m_iValidPoseCount++;
			m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
			if (m_rDevClassChar[nDevice] == 0)
			{
				switch (m_pHMD->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'G'; break;
				case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
				default:                                       m_rDevClassChar[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];
		}
	}

	if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
		m_mat4HMDPose = inverse(m_mat4HMDPose);
	}
}

mat4 Renderer::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
{
	mat4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	return matrixObj;
}


///////////
// CGRENDERMODEL
///////////


//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}


//-----------------------------------------------------------------------------
// Purpose: Finds a render model we've already loaded or loads a new one
//-----------------------------------------------------------------------------
CGLRenderModel* Renderer::FindOrLoadRenderModel(const char* pchRenderModelName)
{
	CGLRenderModel *pRenderModel = NULL;
	for (std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++)
	{
		if (!stricmp((*i)->GetName().c_str(), pchRenderModelName))
		{
			pRenderModel = *i;
			break;
		}
	}

	// load the model if we didn't find one
	if (!pRenderModel)
	{
		vr::RenderModel_t *pModel;
		vr::EVRRenderModelError error;
		while (1)
		{
			error = vr::VRRenderModels()->LoadRenderModel_Async(pchRenderModelName, &pModel);
			if (error != vr::VRRenderModelError_Loading)
				break;
			_sleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			printf("Unable to load render model %s - %s\n", pchRenderModelName, vr::VRRenderModels()->GetRenderModelErrorNameFromEnum(error));
			return NULL; // move on to the next tracked device
		}

		vr::RenderModel_TextureMap_t *pTexture;
		while (1)
		{
			error = vr::VRRenderModels()->LoadTexture_Async(pModel->diffuseTextureId, &pTexture);
			if (error != vr::VRRenderModelError_Loading)
				break;

			_sleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			printf("Unable to load render texture id:%d for render model %s\n", pModel->diffuseTextureId, pchRenderModelName);
			vr::VRRenderModels()->FreeRenderModel(pModel);
			return NULL; // move on to the next tracked device
		}

		pRenderModel = new CGLRenderModel(pchRenderModelName);
		if (!pRenderModel->BInit(*pModel, *pTexture))
		{
			printf("Unable to create GL model from render model %s\n", pchRenderModelName);
			delete pRenderModel;
			pRenderModel = NULL;
		}
		else
		{
			m_vecRenderModels.push_back(pRenderModel);
		}
		vr::VRRenderModels()->FreeRenderModel(pModel);
		vr::VRRenderModels()->FreeTexture(pTexture);
	}
	return pRenderModel;
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL a Render Model for a single tracked device
//-----------------------------------------------------------------------------
void Renderer::SetupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex)
{
	if (unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount)
		return;

	// try to find a model we've already set up
	std::string sRenderModelName = GetTrackedDeviceString(m_pHMD, unTrackedDeviceIndex, vr::Prop_RenderModelName_String);
	CGLRenderModel *pRenderModel = FindOrLoadRenderModel(sRenderModelName.c_str());
	if (!pRenderModel)
	{
		std::string sTrackingSystemName = GetTrackedDeviceString(m_pHMD, unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String);
		printf("Unable to load render model for tracked device %d (%s.%s)", unTrackedDeviceIndex, sTrackingSystemName.c_str(), sRenderModelName.c_str());
	}
	else
	{
		m_rTrackedDeviceToRenderModel[unTrackedDeviceIndex] = pRenderModel;
		m_rbShowTrackedDevice[unTrackedDeviceIndex] = true;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
void Renderer::SetupRenderModels()
{
	memset(m_rTrackedDeviceToRenderModel, 0, sizeof(m_rTrackedDeviceToRenderModel));

	if (!m_pHMD)
		return;

	for (uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_pHMD->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		SetupRenderModelForTrackedDevice(unTrackedDevice);
	}

}

bool Renderer::CreateAllShaders()
{
	m_unSceneProgramID = CompileGLShader(
		"Scene",

		// Vertex Shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 v2UVcoordsIn;\n"
		"layout(location = 2) in vec3 v3NormalIn;\n"
		"out vec2 v2UVcoords;\n"
		"void main()\n"
		"{\n"
		"	v2UVcoords = v2UVcoordsIn;\n"
		"	gl_Position = matrix * position;\n"
		"}\n",

		// Fragment Shader
		"#version 410 core\n"
		"uniform sampler2D mytexture;\n"
		"in vec2 v2UVcoords;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = vec4(1,1,1,1);//texture(mytexture, v2UVcoords);\n"
		"}\n"
	);
	m_nSceneMatrixLocation = glGetUniformLocation(m_unSceneProgramID, "matrix");
	if (m_nSceneMatrixLocation == -1)
	{
		printf("Unable to find matrix uniform in scene shader\n");
		return false;
	}

	m_unControllerTransformProgramID = CompileGLShader(
		"Controller",

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3ColorIn;\n"
		"out vec4 v4Color;\n"
		"void main()\n"
		"{\n"
		"	v4Color.xyz = v3ColorIn; v4Color.a = 1.0;\n"
		"	gl_Position = matrix * position;\n"
		"}\n",

		// fragment shader
		"#version 410\n"
		"in vec4 v4Color;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = v4Color;\n"
		"}\n"
	);
	m_nControllerMatrixLocation = glGetUniformLocation(m_unControllerTransformProgramID, "matrix");
	if (m_nControllerMatrixLocation == -1)
	{
		printf("Unable to find matrix uniform in controller shader\n");
		return false;
	}

	m_unRenderModelProgramID = CompileGLShader(
		"render model",

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3NormalIn;\n"
		"layout(location = 2) in vec2 v2TexCoordsIn;\n"
		"out vec2 v2TexCoord;\n"
		"void main()\n"
		"{\n"
		"	v2TexCoord = v2TexCoordsIn;\n"
		"	gl_Position = matrix * vec4(position.xyz, 1);\n"
		"}\n",

		//fragment shader
		"#version 410 core\n"
		"uniform sampler2D diffuse;\n"
		"in vec2 v2TexCoord;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = texture( diffuse, v2TexCoord);\n"
		"}\n"

	);
	m_nRenderModelMatrixLocation = glGetUniformLocation(m_unRenderModelProgramID, "matrix");
	if (m_nRenderModelMatrixLocation == -1)
	{
		printf("Unable to find matrix uniform in render model shader\n");
		return false;
	}

	m_unCompanionWindowProgramID = CompileGLShader(
		"CompanionWindow",

		// vertex shader
		"#version 410 core\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 v2UVIn;\n"
		"noperspective out vec2 v2UV;\n"
		"void main()\n"
		"{\n"
		"	v2UV = v2UVIn;\n"
		"	gl_Position = position;\n"
		"}\n",

		// fragment shader
		"#version 410 core\n"
		"uniform sampler2D mytexture;\n"
		"noperspective in vec2 v2UV;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"		outputColor = texture(mytexture, v2UV);\n"
		"}\n"
	);

	return m_unSceneProgramID != 0
		&& m_unControllerTransformProgramID != 0
		&& m_unRenderModelProgramID != 0
		&& m_unCompanionWindowProgramID != 0;
}

//-----------------------------------------------------------------------------
// Purpose: Compiles a GL shader program and returns the handle. Returns 0 if
//			the shader couldn't be compiled for some reason.
//-----------------------------------------------------------------------------
GLuint Renderer::CompileGLShader(const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader)
{
	GLuint unProgramID = glCreateProgram();

	GLuint nSceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(nSceneVertexShader, 1, &pchVertexShader, NULL);
	glCompileShader(nSceneVertexShader);

	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv(nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if (vShaderCompiled != GL_TRUE)
	{
		printf("%s - Unable to compile vertex shader %d!\n", pchShaderName, nSceneVertexShader);
		glDeleteProgram(unProgramID);
		glDeleteShader(nSceneVertexShader);
		return 0;
	}
	glAttachShader(unProgramID, nSceneVertexShader);
	glDeleteShader(nSceneVertexShader); // the program hangs onto this once it's attached

	GLuint  nSceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(nSceneFragmentShader, 1, &pchFragmentShader, NULL);
	glCompileShader(nSceneFragmentShader);

	GLint fShaderCompiled = GL_FALSE;
	glGetShaderiv(nSceneFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
	if (fShaderCompiled != GL_TRUE)
	{
		printf("%s - Unable to compile fragment shader %d!\n", pchShaderName, nSceneFragmentShader);
		glDeleteProgram(unProgramID);
		glDeleteShader(nSceneFragmentShader);
		return 0;
	}

	glAttachShader(unProgramID, nSceneFragmentShader);
	glDeleteShader(nSceneFragmentShader); // the program hangs onto this once it's attached

	glLinkProgram(unProgramID);

	GLint programSuccess = GL_TRUE;
	glGetProgramiv(unProgramID, GL_LINK_STATUS, &programSuccess);
	if (programSuccess != GL_TRUE)
	{
		printf("%s - Error linking program %d!\n", pchShaderName, unProgramID);
		glDeleteProgram(unProgramID);
		return 0;
	}

	glUseProgram(unProgramID);
	glUseProgram(0);

	return unProgramID;
}




//-----------------------------------------------------------------------------
// Purpose: create a sea of cubes
//-----------------------------------------------------------------------------
void Renderer::SetupScene()
{
	if (!m_pHMD)
		return;

	std::vector<float> vertdataarray;

	mat4 matScale = glm::scale(mat4(), vec3(m_fScale, m_fScale, m_fScale));
	mat4 matTransform = glm::translate(mat4(), vec3(
		-((float)m_iSceneVolumeWidth * m_fScaleSpacing) / 2.f,
		-((float)m_iSceneVolumeHeight * m_fScaleSpacing) / 2.f,
		-((float)m_iSceneVolumeDepth * m_fScaleSpacing) / 2.f)
	);

	mat4 mat = matScale * matTransform;
	vec3 pos;

	for (int z = 0; z< m_iSceneVolumeDepth; z++)
	{
		for (int y = 0; y< m_iSceneVolumeHeight; y++)
		{
			for (int x = 0; x< m_iSceneVolumeWidth; x++)
			{
				AddCubeToScene(mat, vertdataarray);
				pos = vec3(m_fScaleSpacing, 0, 0);
				mat = glm::translate(mat , pos);
			}
			pos = vec3(-((float)m_iSceneVolumeWidth) * m_fScaleSpacing, m_fScaleSpacing, 0);
			mat = glm::translate(mat, pos);
		}
		pos = vec3(0, -((float)m_iSceneVolumeHeight) * m_fScaleSpacing, m_fScaleSpacing);
		mat = glm::translate(mat, pos);
	}
	m_uiVertcount = vertdataarray.size() / 5;

	glGenVertexArrays(1, &m_unSceneVAO);
	glBindVertexArray(m_unSceneVAO);

	glGenBuffers(1, &m_glSceneVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_glSceneVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STATIC_DRAW);

	GLsizei stride = sizeof(VertexDataScene);
	uintptr_t offset = 0;

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

	offset += sizeof(vec3);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::AddCubeVertex(float fl0, float fl1, float fl2, float fl3, float fl4, std::vector<float> &vertdata)
{
	vertdata.push_back(fl0);
	vertdata.push_back(fl1);
	vertdata.push_back(fl2);
	vertdata.push_back(fl3);
	vertdata.push_back(fl4);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::AddCubeToScene(mat4 mat, std::vector<float> &vertdata)
{
	// Matrix4 mat( outermat.data() );

	vec4 A = mat * vec4(0, 0, 0, 1);
	vec4 B = mat * vec4(1, 0, 0, 1);
	vec4 C = mat * vec4(1, 1, 0, 1);
	vec4 D = mat * vec4(0, 1, 0, 1);
	vec4 E = mat * vec4(0, 0, 1, 1);
	vec4 F = mat * vec4(1, 0, 1, 1);
	vec4 G = mat * vec4(1, 1, 1, 1);
	vec4 H = mat * vec4(0, 1, 1, 1);

	// triangles instead of quads
	AddCubeVertex(E.x, E.y, E.z, 0, 1, vertdata); //Front
	AddCubeVertex(F.x, F.y, F.z, 1, 1, vertdata);
	AddCubeVertex(G.x, G.y, G.z, 1, 0, vertdata);
	AddCubeVertex(G.x, G.y, G.z, 1, 0, vertdata);
	AddCubeVertex(H.x, H.y, H.z, 0, 0, vertdata);
	AddCubeVertex(E.x, E.y, E.z, 0, 1, vertdata);

	AddCubeVertex(B.x, B.y, B.z, 0, 1, vertdata); //Back
	AddCubeVertex(A.x, A.y, A.z, 1, 1, vertdata);
	AddCubeVertex(D.x, D.y, D.z, 1, 0, vertdata);
	AddCubeVertex(D.x, D.y, D.z, 1, 0, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 0, 0, vertdata);
	AddCubeVertex(B.x, B.y, B.z, 0, 1, vertdata);

	AddCubeVertex(H.x, H.y, H.z, 0, 1, vertdata); //Top
	AddCubeVertex(G.x, G.y, G.z, 1, 1, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 1, 0, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 1, 0, vertdata);
	AddCubeVertex(D.x, D.y, D.z, 0, 0, vertdata);
	AddCubeVertex(H.x, H.y, H.z, 0, 1, vertdata);

	AddCubeVertex(A.x, A.y, A.z, 0, 1, vertdata); //Bottom
	AddCubeVertex(B.x, B.y, B.z, 1, 1, vertdata);
	AddCubeVertex(F.x, F.y, F.z, 1, 0, vertdata);
	AddCubeVertex(F.x, F.y, F.z, 1, 0, vertdata);
	AddCubeVertex(E.x, E.y, E.z, 0, 0, vertdata);
	AddCubeVertex(A.x, A.y, A.z, 0, 1, vertdata);

	AddCubeVertex(A.x, A.y, A.z, 0, 1, vertdata); //Left
	AddCubeVertex(E.x, E.y, E.z, 1, 1, vertdata);
	AddCubeVertex(H.x, H.y, H.z, 1, 0, vertdata);
	AddCubeVertex(H.x, H.y, H.z, 1, 0, vertdata);
	AddCubeVertex(D.x, D.y, D.z, 0, 0, vertdata);
	AddCubeVertex(A.x, A.y, A.z, 0, 1, vertdata);

	AddCubeVertex(F.x, F.y, F.z, 0, 1, vertdata); //Right
	AddCubeVertex(B.x, B.y, B.z, 1, 1, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 1, 0, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 1, 0, vertdata);
	AddCubeVertex(G.x, G.y, G.z, 0, 0, vertdata);
	AddCubeVertex(F.x, F.y, F.z, 0, 1, vertdata);
}








//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
CGLRenderModel::CGLRenderModel(const std::string & sRenderModelName)
	: m_sModelName(sRenderModelName)
{
	m_glIndexBuffer = 0;
	m_glVertArray = 0;
	m_glVertBuffer = 0;
	m_glTexture = 0;
}


CGLRenderModel::~CGLRenderModel()
{
	Cleanup();
}


//-----------------------------------------------------------------------------
// Purpose: Allocates and populates the GL resources for a render model
//-----------------------------------------------------------------------------
bool CGLRenderModel::BInit(const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture)
{
	// create and bind a VAO to hold state for this model
	glGenVertexArrays(1, &m_glVertArray);
	glBindVertexArray(m_glVertArray);

	// Populate a vertex buffer
	glGenBuffers(1, &m_glVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vr::RenderModel_Vertex_t) * vrModel.unVertexCount, vrModel.rVertexData, GL_STATIC_DRAW);

	// Identify the components in the vertex buffer
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, vPosition));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, vNormal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, rfTextureCoord));

	// Create and populate the index buffer
	glGenBuffers(1, &m_glIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * vrModel.unTriangleCount * 3, vrModel.rIndexData, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// create and populate the texture
	glGenTextures(1, &m_glTexture);
	glBindTexture(GL_TEXTURE_2D, m_glTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vrDiffuseTexture.unWidth, vrDiffuseTexture.unHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, vrDiffuseTexture.rubTextureMapData);

	// If this renders black ask McJohn what's wrong.
	(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

	glBindTexture(GL_TEXTURE_2D, 0);

	m_unVertexCount = vrModel.unTriangleCount * 3;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Frees the GL resources for a render model
//-----------------------------------------------------------------------------
void CGLRenderModel::Cleanup()
{
	if (m_glVertBuffer)
	{
		glDeleteBuffers(1, &m_glIndexBuffer);
		glDeleteVertexArrays(1, &m_glVertArray);
		glDeleteBuffers(1, &m_glVertBuffer);
		m_glIndexBuffer = 0;
		m_glVertArray = 0;
		m_glVertBuffer = 0;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Draws the render model
//-----------------------------------------------------------------------------
void CGLRenderModel::Draw()
{
	glBindVertexArray(m_glVertArray);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_glTexture);

	glDrawElements(GL_TRIANGLES, m_unVertexCount, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
}


void Renderer::RenderControllerAxes()
{
	// don't draw controllers if somebody else has input focus
	if (m_pHMD->IsInputFocusCapturedByAnotherProcess())
		return;

	std::vector<float> vertdataarray;

	m_uiControllerVertcount = 0;
	m_iTrackedControllerCount = 0;

	for (vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice)
	{
		if (!m_pHMD->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		if (m_pHMD->GetTrackedDeviceClass(unTrackedDevice) != vr::TrackedDeviceClass_Controller)
			continue;

		m_iTrackedControllerCount += 1;

		if (!m_rTrackedDevicePose[unTrackedDevice].bPoseIsValid)
			continue;

		const mat4 & mat = m_rmat4DevicePose[unTrackedDevice];

		vec4 center = mat * vec4(0, 0, 0, 1);

		for (int i = 0; i < 3; ++i)
		{
			vec3 color(0, 0, 0);
			vec4 point(0, 0, 0, 1);
			point[i] += 0.05f;  // offset in X, Y, Z
			color[i] = 1.0;  // R, G, B
			point = mat * point;
			vertdataarray.push_back(center.x);
			vertdataarray.push_back(center.y);
			vertdataarray.push_back(center.z);

			vertdataarray.push_back(color.x);
			vertdataarray.push_back(color.y);
			vertdataarray.push_back(color.z);

			vertdataarray.push_back(point.x);
			vertdataarray.push_back(point.y);
			vertdataarray.push_back(point.z);

			vertdataarray.push_back(color.x);
			vertdataarray.push_back(color.y);
			vertdataarray.push_back(color.z);

			m_uiControllerVertcount += 2;
		}

		vec4 start = mat * vec4(0, 0, -0.02f, 1);
		vec4 end = mat * vec4(0, 0, -39.f, 1);
		vec3 color(.92f, .92f, .71f);

		vertdataarray.push_back(start.x); vertdataarray.push_back(start.y); vertdataarray.push_back(start.z);
		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

		vertdataarray.push_back(end.x); vertdataarray.push_back(end.y); vertdataarray.push_back(end.z);
		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);
		m_uiControllerVertcount += 2;
	}

	// Setup the VAO the first time through.
	if (m_unControllerVAO == 0)
	{
		glGenVertexArrays(1, &m_unControllerVAO);
		glBindVertexArray(m_unControllerVAO);

		glGenBuffers(1, &m_glControllerVertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_glControllerVertBuffer);

		GLuint stride = 2 * 3 * sizeof(float);
		uintptr_t offset = 0;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof(vec3);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray(0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_glControllerVertBuffer);

	// set vertex data if we have some
	if (vertdataarray.size() > 0)
	{
		//$ TODO: Use glBufferSubData for this...
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW);
	}
}