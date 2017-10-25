#pragma once

#include "impl/GL.hpp"
#include <string>
#include <openvr.h>

class VRRenderModel
{
public:
	VRRenderModel(const std::string& model_name);
	~VRRenderModel();

	bool initialize(const vr::RenderModel_t& vr_model, const vr::RenderModel_TextureMap_t& vr_diffuse_texture);
	void cleanup();
	void draw();
	const std::string& get_name() const { return _name; }

private:
	GLuint m_glVertBuffer;
	GLuint m_glIndexBuffer;
	GLuint m_glVertArray;
	GLuint m_glTexture;
	GLsizei _vertex_count;
	std::string _name;
};
