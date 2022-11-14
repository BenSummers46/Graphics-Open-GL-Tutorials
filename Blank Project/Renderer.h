#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "../nclgl/Frustum.h"

class Camera;
class Shader;
class HeightMap;

class Renderer : public OGLRenderer	{
public:
	Renderer(Window &parent);
	 ~Renderer(void);
	 void RenderScene()	  override;
	 void UpdateScene(float msec)	override;
protected:
	void DrawHeightMap();
	void DrawSkyBox();
	void DrawWater();
	void DrawTower();
	void DrawTree();
	void LoadTextures();

	float sceneTime;

	Shader* lightShader;
	Shader* skyboxShader;
	Shader* waterShader;
	Shader* towerShader;
	Shader* mainTreeShader;

	HeightMap* heightMap;
	HeightMap* waterMap;
	Mesh* quad;
	Mesh* tower;
	Mesh* tree;

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
	GLuint waterBump;
	GLuint towerTex;
	GLuint towerBump;
	GLuint mainTreeTex;

	Frustum frameFrustum;

};
