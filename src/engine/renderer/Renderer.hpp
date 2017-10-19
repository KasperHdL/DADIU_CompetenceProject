#pragma once

#include <SDL_video.h>
#include "glm/glm.hpp"

#include "impl/GL.hpp"

#include <iostream>
#include <vector>

#include <openvr.h>

#include <engine/utils/AssetManager.hpp>

#include <engine/renderer/Shader.hpp>
#include <engine/renderer/Mesh.hpp>
#include <engine/renderer/Light.hpp>



class Camera;
class Entity;
class DebugInterface;
class Renderer{
public:
    Renderer();
    ~Renderer();

    bool initialize(SDL_Window* window, int screen_width, int screen_height);

    void render(float delta_time);

    DebugInterface* debug;

    Camera* camera;

    vec4 ambient_light;

    //Shader
    Shader* scene_shader;
	Shader* window_shader;
	Shader* test_shader;

    //OpenGL
    SDL_Window* window;
    SDL_GLContext glcontext;


    float time = 0;

    int max_shadow_maps = 1;
    int shadow_width = 3000;
    int shadow_height = 3000;

    int screen_width;
    int screen_height;
    static Renderer* instance;

	mat4 current_model_matrix;
	mat4 current_view_matrix;
	mat4 current_projection_matrix;

	//VR

	mat4 m_mat4ProjectionCenter;
	mat4 m_mat4ProjectionLeft;
	mat4 m_mat4ProjectionRight;
	mat4 m_mat4eyePosLeft;
	mat4 m_mat4eyePosRight;

	mat4 m_mat4HMDPose;


	uint32_t vr_screen_width;
	uint32_t vr_screen_height;
	float vr_nearclip = 0.1f;
	float vr_farclip = 30;

	struct FrameBufferDescription {
		GLuint depthbuffer_id;
		GLuint rendertexture_id;
		GLuint framebuffer_id;
		GLuint resolve_texture_id;
		GLuint resolve_framebuffer_id;
	};

	struct VertexDataScene
	{
		vec3 position;
		vec2 texCoord;
	};

	struct VertexDataWindow
	{
		vec2 position;
		vec2 texCoord;

		VertexDataWindow(const vec2 & pos, const vec2 tex) : position(pos), texCoord(tex) {	}
	};

	FrameBufferDescription left_eye_description;
	FrameBufferDescription right_eye_description;

	vr::IVRSystem *m_pHMD = nullptr;
	vr::IVRRenderModels *m_pRenderModels = nullptr;
	

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	mat4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
	bool m_rbShowTrackedDevice[vr::k_unMaxTrackedDeviceCount];


	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;

	std::string m_strPoseClasses;                            // what classes we saw poses for this frame
	char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class


	GLuint m_unCompanionWindowVAO;
	GLuint m_glCompanionWindowIDVertBuffer;
	GLuint m_glCompanionWindowIDIndexBuffer;
	unsigned int m_uiCompanionWindowIndexSize;


	mat4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
	mat4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);
	mat4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye);
	void UpdateHMDMatrixPose();

	void RenderScene(vr::Hmd_Eye nEye);
	mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);

	bool SetupStereoRenderTargets();
	void SetupCameras();
	void SetupCompanionWindow();

	void RenderCompanionWindow();
	void RenderFrame(float delta_time);


	vr::IVRRenderModels *m_pRenderModels;

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	mat4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
	bool m_rbShowTrackedDevice[vr::k_unMaxTrackedDeviceCount];

	std::vector< CGLRenderModel * > m_vecRenderModels;
	CGLRenderModel *m_rTrackedDeviceToRenderModel[vr::k_unMaxTrackedDeviceCount];

	

	CGLRenderModel* FindOrLoadRenderModel(const char *pchRenderModelName);
	void SetupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	void SetupRenderModels();
	void ProcessVREvent(const vr::VREvent_t & event);

	void RenderControllerAxes();

	GLuint m_glControllerVertBuffer;
	GLuint m_unControllerVAO;
	unsigned int m_uiControllerVertcount;

private:

	bool _create_framebuffer(int width, int height, FrameBufferDescription &framebuffer);
	
	void _render_stereo_targets();

    void _render_entity(Entity* entity);
    void _render_pool(DynamicPool<Entity*> pool);
	void _render_scene();
};

class CGLRenderModel
{
public:
	CGLRenderModel(const std::string & sRenderModelName);
	~CGLRenderModel();

	bool BInit(const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture);
	void Cleanup();
	void Draw();
	const std::string & GetName() const { return m_sModelName; }

private:
	GLuint m_glVertBuffer;
	GLuint m_glIndexBuffer;
	GLuint m_glVertArray;
	GLuint m_glTexture;
	GLsizei m_unVertexCount;
	std::string m_sModelName;
};