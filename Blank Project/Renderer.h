#pragma once
#include "../NCLGL/OGLRenderer.h"

class Camera;
class Shader;
class HeightMap;

class Renderer : public OGLRenderer	{
public:
	Renderer(Window &parent);
	 ~Renderer(void);
	 void RenderScene()				override;
	 void UpdateScene(float msec)	override;
protected:
	void DrawHeightMap();
	void DrawSkyBox();

	Shader* lightShader;
	Shader* skyboxShader;

	HeightMap* heightMap;
	HeightMap* waterMap;
	Mesh* quad;
	//Mesh* plane;

	Light* light;
	Camera* camera;

	GLuint cubeMap;
	GLuint earthTex;
	GLuint earthBump;
	GLuint forestTex;
	GLuint forestBump;
	GLuint coastTex;
	GLuint coastBump;
	GLuint snowTex;
	GLuint snowBump;
	//GLuint waterTex;

};
