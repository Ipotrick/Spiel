#pragma once

#include "OGLShaderUtil.hpp"
#include "OGLTextureRenderBuffer.hpp"

struct PassShaderVertex {
	static int constexpr FLOAT_SIZE{ 4 };
	constexpr PassShaderVertex() :
		data{ 0 }
	{
	}
	constexpr PassShaderVertex(Vec2 corner, Vec2 uv) :
		cornerPos{ corner }, samplerCoord{ uv }
	{ }
	union {
		struct {
			Vec2 cornerPos;
			Vec2 samplerCoord;
		};
		float data[FLOAT_SIZE];
	};
};

inline constexpr GLuint PASS_SHADER_INDICES[6] = { 0,1,2,1,2,3 };

inline constexpr PassShaderVertex PASS_SHADER_VERTECIES[4] = {
	PassShaderVertex{ idToCorner(0, { -1,-1 }, { 1,1 }), idToCorner(0, { 0, 0 }, { 1,1 }) },
	PassShaderVertex{ idToCorner(1, { -1,-1 }, { 1,1 }), idToCorner(1, { 0, 0 }, { 1,1 }) },
	PassShaderVertex{ idToCorner(2, { -1,-1 }, { 1,1 }), idToCorner(2, { 0, 0 }, { 1,1 }) },
	PassShaderVertex{ idToCorner(3, { -1,-1 }, { 1,1 }), idToCorner(3, { 0, 0 }, { 1,1 }) },
};

class OGLPassShader {
public:
	/**
	 * creates needed openGL ressources.
	 * The shader is NOT usable prior to initialization!
	 * The shader source code is compiled with this function.
	 * 
	 * \param customFragmentShader one can set the code for a custom fragment shader to manipulate the texture for example.
	 */
	void initialize(std::string const& customFragmentShader);
	/**
	 * deletes all openGL ressources.
	 * After calling this, the shader will become unusable.
	 */
	void reset();
	/**
	 * renderes to the set fbo.
	 *
	 * \param textureGLID texture id that can be used as sampling data in fragment shader
	 * \param fboGLID framebuffer object openGL identificator
	 * \param fboWidth width of framebuffer
	 * \param fboHeight height of framebuffer
	 */
	void renderTexToFBO(GLuint textureGLID, GLuint fboGLID, uint32_t fboWidth, uint32_t fboHeight);
	void renderTexToFBO(GLuint textureGLID, OGLTexFrameBuffer& fbo);
	void renderToFBO(OGLTexFrameBuffer& fbo);

	void setViewPortOffset(Vec2 offset);

	void bind();
private:
	Vec2 viewPortOffset{ 0,0 };

	bool bInitialized{ false };
	GLuint vao = -1;
	GLuint vbo = -1;
	GLuint program = -1;
};