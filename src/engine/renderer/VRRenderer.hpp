#pragma once

/////
// Forked hellovr_opengl (Copyright Valve Corperation)
// adjusted to fit into the engine
//

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>

#include "impl/GL.hpp"
#include <SDL_video.h>
#include <vector>

#include <GL/glu.h>

#include <stdio.h>
#include <string>
#include <cstdlib>

#include <openvr.h>

#include <engine/renderer/Mesh.hpp>
#include <engine/renderer/Shader.hpp>
#include <engine/renderer/VRRenderModel.hpp>
#include <engine/utils/AssetManager.hpp>
#include <debug/DebugInterface.hpp>
#include <engine/Input.hpp>
#include <engine/God.hpp>

#include "glm/glm.hpp"


#if defined(POSIX)
#include "unistd.h"
#endif

#define APIENTRY

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

void ThreadSleep(unsigned long nMilliseconds)
{
#if defined(_WIN32)
	_sleep(nMilliseconds);
#elif defined(POSIX)
	usleep(nMilliseconds * 1000);
#endif
}


static bool g_bPrintf = true;

//-----------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
class VRRenderer
{
private:
	enum Eye
	{
		Left,
		Right,
		Debug,
	};

public:
	VRRenderer();
	virtual ~VRRenderer();

	bool initialize(SDL_Window* window, DebugInterface* debug);
	bool initialize_gl();
	bool initialize_compositor();


	void shutdown();

	void handle_input();
	void process_VREvent(const vr::VREvent_t& event);
	void render(float dt);

	void render_controller_axis();

	void setup_render_models();
	bool setup_stereo_render_targets();
	void setup_companion_window();
	void setup_cameras();

	void render_stereo_targets();
	void render_companion_window();
	void render_scene(Eye eye);

	mat4 get_hmd_projection_eye(vr::Hmd_Eye eye);
	mat4 get_hmd_pose_eye(vr::Hmd_Eye eye);

	mat4 convert_HmdMatrix_to_mat4(const vr::HmdMatrix34_t& matrix);

	void create_all_shaders();

	void setup_render_model_for_tracked_device(vr::TrackedDeviceIndex_t tracked_device_index);
	VRRenderModel* find_or_load_render_model(const char* render_model_name);

	mat4 current_model_transform;
	mat4 current_view_transform;
	mat4 current_projection_transform;

	DebugInterface* debug;

	Shader* scene_shader;
	Shader* window_shader;
	Shader* render_model_shader;
	Shader* transform_shader;

	vr::IVRSystem* hmd;
	vr::IVRChaperone* chaperone;

private:
	void _render_pool(DynamicPool<Entity*> pool, bool bind_mesh = false);
	
	bool _mode_debug_opengl;
	bool _mode_verbose;
	bool _mode_performance;
	bool _mode_vblank;
	bool _mode_gl_finish_hack;

	vr::IVRRenderModels* render_model_interface;
	std::string driver_name;
	std::string display_name;

	bool show_tracked_device[vr::k_unMaxTrackedDeviceCount];

private: // SDL bookkeeping
	SDL_Window* companion_window;

	uint32_t companion_window_width;
	uint32_t companion_window_height;

	SDL_GLContext context;

private: // OpenGL bookkeeping
	
	float near_clip;
	float far_clip;


	GLuint companion_window_vao;
	GLuint companion_window_id_vert_buffer;
	GLuint companion_window_id_index_buffer;
	unsigned int companion_window_index_size;

	GLuint controller_vert_buffer;
	GLuint controller_vao;
	unsigned int controller_vert_count;

	mat4 hmd_pose_matrix;
	mat4 eye_pos_left_matrix;
	mat4 eye_pos_right_matrix;

	mat4 projection_left_matrix;
	mat4 projection_right_matrix;


	struct VertexDataScene
	{
		vec3 position;
		vec2 texture_coord;
	};

	struct VertexDataWindow
	{
		vec2 position;
		vec2 texture_coord;

		VertexDataWindow(const vec2& pos, const vec2 tex) : position(pos), texture_coord(tex) {	}
	};

	struct FramebufferDescription
	{
		GLuint depth_buffer_id;
		GLuint render_texture_id;
		GLuint render_framebuffer_id;
		GLuint resolve_texture_id;
		GLuint resolve_framebuffer_id;
	};
	FramebufferDescription left_eye_description;
	FramebufferDescription right_eye_description;

	bool CreateFrameBuffer(int width, int height, FramebufferDescription &framebuffer_description);

	uint32_t render_width;
	uint32_t render_height;

	std::vector<VRRenderModel*> render_models;
	VRRenderModel* tracked_device_to_render_model[vr::k_unMaxTrackedDeviceCount];
};

//-----------------------------------------------------------------------------
// Purpose: Outputs a set of optional arguments to debugging output, using
//          the printf format setting specified in fmt*.
//-----------------------------------------------------------------------------
void dprintf(const char *fmt, ...)
{
	va_list args;
	char buffer[2048];

	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

	if (g_bPrintf)
		printf("%s", buffer);

}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
VRRenderer::VRRenderer()
	: companion_window(NULL)
	, context(NULL)

	, companion_window_width(1280)
	, companion_window_height(720)

	, hmd(NULL)
	, render_model_interface(NULL)

	, _mode_debug_opengl(false)
	, _mode_verbose(false)
	, _mode_performance(false)
	, _mode_vblank(false)
	, _mode_gl_finish_hack(true)
	
	, controller_vert_buffer(0)
	, controller_vao(0)
{

		_mode_debug_opengl = true;
};


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
VRRenderer::~VRRenderer()
{
	// work is done in Shutdown
	dprintf("Shutdown");
}


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
// Purpose:
//-----------------------------------------------------------------------------
bool VRRenderer::initialize(SDL_Window* window, DebugInterface* debug)
{
	this->debug = debug;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	// Loading the SteamVR Runtime
	vr::EVRInitError eError = vr::VRInitError_None;
	hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);

	if (eError != vr::VRInitError_None)
	{
		hmd = NULL;
		char buf[1024];
		sprintf_s(buf, sizeof(buf), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL);
		return false;
	}


	render_model_interface = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
	if (!render_model_interface)
	{
		hmd = NULL;
		vr::VR_Shutdown();

		char buf[1024];
		sprintf_s(buf, sizeof(buf), "Unable to get render model interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL);
		return false;
	}

	int nWindowPosX = 700;
	int nWindowPosY = 100;
	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	if (_mode_debug_opengl)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	companion_window = window;
	if (companion_window == NULL)
	{
		printf("%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	context = SDL_GL_CreateContext(companion_window);
	if (context == NULL)
	{
		printf("%s - OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	glewExperimental = GL_TRUE;
	GLenum nGlewError = glewInit();
	if (nGlewError != GLEW_OK)
	{
		printf("%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString(nGlewError));
		return false;
	}
	glGetError(); // to clear the error caused deep in GLEW

	if (SDL_GL_SetSwapInterval(_mode_vblank ? 1 : 0) < 0)
	{
		printf("%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	near_clip = 0.1f;
	far_clip = 30.0f;

	// 		m_MillisecondsTimer.start(1, this);
	// 		m_SecondsTimer.start(1000, this);

	if (!initialize_gl())
	{
		printf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
		return false;
	}

	if (!initialize_compositor())
	{
		printf("%s - Failed to initialize VR Compositor!\n", __FUNCTION__);
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Initialize OpenGL. Returns true if OpenGL has been successfully
//          initialized, false if shaders could not be created.
//          If failure occurred in a module other than shaders, the function
//          may return true or throw an error. 
//-----------------------------------------------------------------------------
bool VRRenderer::initialize_gl()
{
	
	create_all_shaders();

	setup_cameras();
	setup_stereo_render_targets();
	setup_companion_window();
	setup_render_models();

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Initialize Compositor. Returns true if the compositor was
//          successfully initialized, false otherwise.
//-----------------------------------------------------------------------------
bool VRRenderer::initialize_compositor()
{
	vr::EVRInitError peError = vr::VRInitError_None;

	if (!vr::VRCompositor())
	{
		printf("Compositor initialization failed. See log file for details\n");
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void VRRenderer::shutdown()
{
	if (hmd)
	{
		vr::VR_Shutdown();
		hmd = NULL;
	}

	for (std::vector<VRRenderModel*>::iterator i = render_models.begin(); i != render_models.end(); i++)
	{
		delete (*i);
	}
	render_models.clear();

	if (context)
	{
		if (_mode_debug_opengl)
		{
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
			glDebugMessageCallback(nullptr, nullptr);
		}

		glDeleteRenderbuffers(1, &left_eye_description.depth_buffer_id);
		glDeleteTextures(1, &left_eye_description.render_texture_id);
		glDeleteFramebuffers(1, &left_eye_description.render_framebuffer_id);
		glDeleteTextures(1, &left_eye_description.resolve_texture_id);
		glDeleteFramebuffers(1, &left_eye_description.resolve_framebuffer_id);

		glDeleteRenderbuffers(1, &right_eye_description.depth_buffer_id);
		glDeleteTextures(1, &right_eye_description.render_texture_id);
		glDeleteFramebuffers(1, &right_eye_description.render_framebuffer_id);
		glDeleteTextures(1, &right_eye_description.resolve_texture_id);
		glDeleteFramebuffers(1, &right_eye_description.resolve_framebuffer_id);

		if (companion_window_vao != 0)
		{
			glDeleteVertexArrays(1, &companion_window_vao);
		}
		
		if (controller_vao != 0)
		{
			glDeleteVertexArrays(1, &controller_vao);
		}
	}

		companion_window = NULL;
	
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void VRRenderer::handle_input()
{
		// Process SteamVR events
	vr::VREvent_t event;
	while (hmd->PollNextEvent(&event, sizeof(event)))
	{
		process_VREvent(event);
	}

	
}


//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------
void VRRenderer::process_VREvent(const vr::VREvent_t & event)
{
	switch (event.eventType)
	{
	case vr::VREvent_TrackedDeviceActivated:
	{
		setup_render_model_for_tracked_device(event.trackedDeviceIndex);
		dprintf("Device %u attached. Setting up render model.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceDeactivated:
	{
		dprintf("Device %u detached.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceUpdated:
	{
		dprintf("Device %u updated.\n", event.trackedDeviceIndex);
	}
	break;
	}
}


void VRRenderer::create_all_shaders()
{
	{ //Scene Shader
		scene_shader = AssetManager::get_shader("shaders/standard");

		scene_shader->init_uniform("model", Shader::Uniform_Type::Mat4);
		scene_shader->init_uniform("view", Shader::Uniform_Type::Mat4);
		scene_shader->init_uniform("projection", Shader::Uniform_Type::Mat4);
		scene_shader->init_uniform("normalMat", Shader::Uniform_Type::Mat3);

		scene_shader->init_uniform("ambientLight", Shader::Uniform_Type::Vec4);
		scene_shader->init_uniform("color", Shader::Uniform_Type::Vec4);


		for (int i = 0; i < 4; i++) {
			scene_shader->init_uniform("lightPosType[" + to_string(i) + "]", Shader::Uniform_Type::Vec4);
			scene_shader->init_uniform("lightColorRange[" + to_string(i) + "]", Shader::Uniform_Type::Vec4);
		}
	}
	{ //Companion Shader
		window_shader = AssetManager::get_shader("shaders/companion_window");

		//window_shader->init_uniform("mytexture", Shader::Uniform_Type::Texture);
	}
	{ //Render Model Shader
		render_model_shader = AssetManager::get_shader("shaders/render_model");

		render_model_shader->init_uniform("matrix", Shader::Uniform_Type::Mat4);
		render_model_shader->init_uniform("diffuse", Shader::Uniform_Type::Texture);
	}
	{ //Transform Shader
		transform_shader = AssetManager::get_shader("shaders/transform");

		transform_shader->init_uniform("matrix", Shader::Uniform_Type::Mat4);
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void VRRenderer::setup_cameras()
{
	projection_left_matrix = get_hmd_projection_eye(vr::Eye_Left);
	projection_right_matrix = get_hmd_projection_eye(vr::Eye_Right);
	eye_pos_left_matrix = get_hmd_pose_eye(vr::Eye_Left);
	eye_pos_right_matrix = get_hmd_pose_eye(vr::Eye_Right);
}


//-----------------------------------------------------------------------------
// Purpose: Creates a frame buffer. Returns true if the buffer was set up.
//          Returns false if the setup failed.
//-----------------------------------------------------------------------------
bool VRRenderer::CreateFrameBuffer(int nWidth, int nHeight, FramebufferDescription &framebufferDesc)
{
	glGenFramebuffers(1, &framebufferDesc.render_framebuffer_id);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.render_framebuffer_id);

	glGenRenderbuffers(1, &framebufferDesc.depth_buffer_id);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.depth_buffer_id);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.depth_buffer_id);

	glGenTextures(1, &framebufferDesc.render_texture_id);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.render_texture_id);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.render_texture_id, 0);

	glGenFramebuffers(1, &framebufferDesc.resolve_framebuffer_id);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.resolve_framebuffer_id);

	glGenTextures(1, &framebufferDesc.resolve_texture_id);
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.resolve_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.resolve_texture_id, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool VRRenderer::setup_stereo_render_targets()
{
	if (!hmd)
		return false;

	hmd->GetRecommendedRenderTargetSize(&render_width, &render_height);

	CreateFrameBuffer(render_width, render_height, left_eye_description);
	CreateFrameBuffer(render_width, render_height, right_eye_description);

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void VRRenderer::setup_companion_window()
{
	if (!hmd)
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
	companion_window_index_size = _countof(vIndices);

	glGenVertexArrays(1, &companion_window_vao);
	glBindVertexArray(companion_window_vao);

	glGenBuffers(1, &companion_window_id_vert_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, companion_window_id_vert_buffer);
	glBufferData(GL_ARRAY_BUFFER, vVerts.size() * sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW);

	glGenBuffers(1, &companion_window_id_index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, companion_window_id_index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, companion_window_index_size * sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void*)offsetof(VertexDataWindow, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void*)offsetof(VertexDataWindow, texture_coord));

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void VRRenderer::render(float dt)
{
	if (hmd)
	{
		render_controller_axis();
		render_stereo_targets();

		if (debug->use_camera) {
			debug->camera->set_viewport(0, 0, companion_window_width, companion_window_height);
			render_scene(Eye::Debug);
		}
		else {
			render_companion_window();
		}


		debug->render(dt);

		vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)left_eye_description.resolve_texture_id, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)right_eye_description.resolve_texture_id, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
	}

	SDL_GL_SwapWindow(companion_window);

	// We want to make sure the glFinish waits for the entire present to complete, not just the submission
	// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void VRRenderer::render_stereo_targets()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_MULTISAMPLE);

	// Left Eye
	glBindFramebuffer(GL_FRAMEBUFFER, left_eye_description.render_framebuffer_id);
	glViewport(0, 0, render_width, render_height);
	render_scene(Eye::Left);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, left_eye_description.render_framebuffer_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, left_eye_description.resolve_framebuffer_id);

	glBlitFramebuffer(0, 0, render_width, render_height, 0, 0, render_width, render_height,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glEnable(GL_MULTISAMPLE);

	// Right Eye
	glBindFramebuffer(GL_FRAMEBUFFER, right_eye_description.render_framebuffer_id);
	glViewport(0, 0, render_width, render_height);
	render_scene(Eye::Right);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, right_eye_description.render_framebuffer_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, right_eye_description.resolve_framebuffer_id);

	glBlitFramebuffer(0, 0, render_width, render_height, 0, 0, render_width, render_height,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


//-----------------------------------------------------------------------------
// Purpose: Renders a scene with respect to nEye.
//-----------------------------------------------------------------------------
void VRRenderer::render_scene(Eye eye)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	scene_shader->use();
		
	current_model_transform = Input::get_hmd_matrix();

	if (eye == Eye::Left)
	{
		current_view_transform = eye_pos_left_matrix;
		current_projection_transform = projection_left_matrix;
	}
	else if (eye == Eye::Right)
	{
		current_view_transform = eye_pos_right_matrix;
		current_projection_transform = projection_right_matrix;
	}
	else if (eye == Eye::Debug) {
		current_model_transform = debug->camera->transform->get_model_transform();
		current_view_transform = debug->camera->view_transform;
		current_projection_transform = debug->camera->projection_transform;
	}

	mat4 mvp = current_projection_transform * current_view_transform * current_model_transform;


	Mesh::get_cube()->bind();
	_render_pool(God::cube_mesh_entities);

	Mesh::get_sphere()->bind();
	_render_pool(God::sphere_mesh_entities);

	Mesh::get_quad()->bind();
	_render_pool(God::quad_mesh_entities);

	_render_pool(God::custom_mesh_entities, true);
	

	bool bIsInputCapturedByAnotherProcess = hmd->IsInputFocusCapturedByAnotherProcess();

	if (!bIsInputCapturedByAnotherProcess)
	{
		// draw the controller axis lines
		transform_shader->use();
		transform_shader->set_uniform("matrix", mvp);

		glBindVertexArray(controller_vao);
		glDrawArrays(GL_LINES, 0, controller_vert_count);
		glBindVertexArray(0);
	}

	// ----- Render Model rendering -----
	render_model_shader->use();

	for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!tracked_device_to_render_model[unTrackedDevice] || !show_tracked_device[unTrackedDevice])
			continue;

		const vr::TrackedDevicePose_t & pose = Input::device_pose[unTrackedDevice];
		if (!pose.bPoseIsValid)
			continue;

		if (bIsInputCapturedByAnotherProcess && Input::device_type[unTrackedDevice] == vr::TrackedDeviceClass_Controller)
			continue;

		mat4 matMVP = mvp * Input::device_matrix[unTrackedDevice];

		render_model_shader->set_uniform("matrix", matMVP);

		tracked_device_to_render_model[unTrackedDevice]->draw();
	}

}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void VRRenderer::render_companion_window()
{
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, companion_window_width, companion_window_height);
	glBindVertexArray(companion_window_vao);
	window_shader->use();

	// render left eye (first half of index array )
	glBindTexture(GL_TEXTURE_2D, left_eye_description.resolve_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDrawElements(GL_TRIANGLES, companion_window_index_size / 2, GL_UNSIGNED_SHORT, 0);

	// render right eye (second half of index array )
	glBindTexture(GL_TEXTURE_2D, right_eye_description.resolve_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDrawElements(GL_TRIANGLES, companion_window_index_size / 2, GL_UNSIGNED_SHORT, (const void *)(uintptr_t)(companion_window_index_size));

	glBindVertexArray(0);

}


void VRRenderer::_render_pool(DynamicPool<Entity*> pool, bool bind_mesh) {
	if (pool.count > 0) {

		for (int i = 0; i < pool.capacity; i++) {
			Entity** p = pool[i];
			if (p != nullptr) {
				Entity* e = *p;
				if (e->is_visible && e->mesh != nullptr) {
					mat4 model = current_model_transform * e->transform->get_model_transform();
					scene_shader->set_uniform("model", model);
					scene_shader->set_uniform("view", current_view_transform);
					scene_shader->set_uniform("projection", current_projection_transform);

					mat3 normalMat = transpose(inverse((glm::mat3)(current_view_transform * model)));
					scene_shader->set_uniform("normalMat", normalMat);


					scene_shader->set_uniform("ambientLight", vec4(0.3f));

					scene_shader->set_uniform("color", e->color);

					//lights
					for (int i = 0; i < 4; i++) {
						Light* l = God::lights[i];
						if (l != nullptr) {

							vec4 light_pos_type = vec4(normalMat * l->position, (int)l->type);
							vec4 light_color_range = vec4(l->color * l->intensity, l->range);

							scene_shader->set_uniform("lightPosType[" + to_string(i) + "]", light_pos_type);
							scene_shader->set_uniform("lightColorRange[" + to_string(i) + "]", light_color_range);

						}
					}
					if (bind_mesh)
						e->mesh->bind();

					int indexCount = (int)e->mesh->indices.size();
					if (indexCount == 0) {
						glDrawArrays((GLenum)e->mesh->topology, 0, e->mesh->vertex_count);
					}
					else {
						glDrawElements((GLenum)e->mesh->topology, indexCount, GL_UNSIGNED_SHORT, 0);
					}
				}
			}
		}
	}
}



//-----------------------------------------------------------------------------
// Purpose: Draw all of the controllers as X/Y/Z lines
//-----------------------------------------------------------------------------
void VRRenderer::render_controller_axis()
{
	// don't draw controllers if somebody else has input focus
	if (hmd->IsInputFocusCapturedByAnotherProcess())
		return;

	std::vector<float> vertdataarray;

	controller_vert_count = 0;

	for (vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice)
	{
		if (!hmd->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		if (hmd->GetTrackedDeviceClass(unTrackedDevice) != vr::TrackedDeviceClass_Controller)
			continue;

		if (!Input::device_pose[unTrackedDevice].bPoseIsValid)
			continue;

		const mat4 & mat = Input::device_matrix[unTrackedDevice];

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

			controller_vert_count += 2;
		}

		vec4 start = mat * vec4(0, 0, -0.02f, 1);
		vec4 end = mat * vec4(0, 0, -39.f, 1);
		vec3 color(.92f, .92f, .71f);

		vertdataarray.push_back(start.x); vertdataarray.push_back(start.y); vertdataarray.push_back(start.z);
		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

		vertdataarray.push_back(end.x); vertdataarray.push_back(end.y); vertdataarray.push_back(end.z);
		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);
		controller_vert_count += 2;
	}

	// Setup the VAO the first time through.
	if (controller_vao == 0)
	{
		glGenVertexArrays(1, &controller_vao);
		glBindVertexArray(controller_vao);

		glGenBuffers(1, &controller_vert_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, controller_vert_buffer);

		GLuint stride = 2 * 3 * sizeof(float);
		uintptr_t offset = 0;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof(vec3);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray(0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, controller_vert_buffer);

	// set vertex data if we have some
	if (vertdataarray.size() > 0)
	{
		//$ TODO: Use glBufferSubData for this...
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW);
	}
}



//-----------------------------------------------------------------------------
// Purpose: Gets a Matrix Projection Eye with respect to nEye.
//-----------------------------------------------------------------------------
mat4 VRRenderer::get_hmd_projection_eye(vr::Hmd_Eye nEye)
{
	if (!hmd)
		return mat4();

	vr::HmdMatrix44_t mat = hmd->GetProjectionMatrix(nEye, near_clip, far_clip);

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
mat4 VRRenderer::get_hmd_pose_eye(vr::Hmd_Eye nEye)
{
	if (!hmd)
		return mat4();

	vr::HmdMatrix34_t matEyeRight = hmd->GetEyeToHeadTransform(nEye);
	mat4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
	);

	return inverse(matrixObj);
}


//-----------------------------------------------------------------------------
// Purpose: Finds a render model we've already loaded or loads a new one
//-----------------------------------------------------------------------------
VRRenderModel *VRRenderer::find_or_load_render_model(const char *pchRenderModelName)
{
	VRRenderModel *pRenderModel = NULL;
	for (std::vector< VRRenderModel * >::iterator i = render_models.begin(); i != render_models.end(); i++)
	{
		if (!stricmp((*i)->get_name().c_str(), pchRenderModelName))
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

			ThreadSleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			dprintf("Unable to load render model %s - %s\n", pchRenderModelName, vr::VRRenderModels()->GetRenderModelErrorNameFromEnum(error));
			return NULL; // move on to the next tracked device
		}

		vr::RenderModel_TextureMap_t *pTexture;
		while (1)
		{
			error = vr::VRRenderModels()->LoadTexture_Async(pModel->diffuseTextureId, &pTexture);
			if (error != vr::VRRenderModelError_Loading)
				break;

			ThreadSleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			dprintf("Unable to load render texture id:%d for render model %s\n", pModel->diffuseTextureId, pchRenderModelName);
			vr::VRRenderModels()->FreeRenderModel(pModel);
			return NULL; // move on to the next tracked device
		}

		pRenderModel = new VRRenderModel(pchRenderModelName);
		if (!pRenderModel->initialize(*pModel, *pTexture))
		{
			dprintf("Unable to create GL model from render model %s\n", pchRenderModelName);
			delete pRenderModel;
			pRenderModel = NULL;
		}
		else
		{
			render_models.push_back(pRenderModel);
		}
		vr::VRRenderModels()->FreeRenderModel(pModel);
		vr::VRRenderModels()->FreeTexture(pTexture);
	}
	return pRenderModel;
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL a Render Model for a single tracked device
//-----------------------------------------------------------------------------
void VRRenderer::setup_render_model_for_tracked_device(vr::TrackedDeviceIndex_t unTrackedDeviceIndex)
{
	if (unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount)
		return;

	// try to find a model we've already set up
	std::string sRenderModelName = GetTrackedDeviceString(hmd, unTrackedDeviceIndex, vr::Prop_RenderModelName_String);
	VRRenderModel *pRenderModel = find_or_load_render_model(sRenderModelName.c_str());
	if (!pRenderModel)
	{
		std::string sTrackingSystemName = GetTrackedDeviceString(hmd, unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String);
		dprintf("Unable to load render model for tracked device %d (%s.%s)", unTrackedDeviceIndex, sTrackingSystemName.c_str(), sRenderModelName.c_str());
	}
	else
	{
		tracked_device_to_render_model[unTrackedDeviceIndex] = pRenderModel;
		show_tracked_device[unTrackedDeviceIndex] = true;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
void VRRenderer::setup_render_models()
{
	memset(tracked_device_to_render_model, 0, sizeof(tracked_device_to_render_model));

	if (!hmd)
		return;

	for (uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!hmd->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		setup_render_model_for_tracked_device(unTrackedDevice);
	}

}


//-----------------------------------------------------------------------------
// Purpose: Converts a SteamVR matrix to our local matrix class
//-----------------------------------------------------------------------------
mat4 VRRenderer::convert_HmdMatrix_to_mat4(const vr::HmdMatrix34_t &matrix)
{
	mat4 mat(
		matrix.m[0][0], matrix.m[1][0], matrix.m[2][0], 0.0,
		matrix.m[0][1], matrix.m[1][1], matrix.m[2][1], 0.0,
		matrix.m[0][2], matrix.m[1][2], matrix.m[2][2], 0.0,
		matrix.m[0][3], matrix.m[1][3], matrix.m[2][3], 1.0f
	);
	return mat;
}
