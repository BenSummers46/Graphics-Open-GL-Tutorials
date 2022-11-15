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
	void CreateMatrixUBO();
	void DrawForest();

	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void DrawNode(SceneNode* n);
	void DrawNodes();
	void ClearNodeLists();

	float sceneTime;

	Shader* lightShader;
	Shader* skyboxShader;
	Shader* waterShader;
	Shader* towerShader;
	Shader* mainTreeShader;
	Shader* forestShader;

	HeightMap* heightMap;
	HeightMap* waterMap;
	Mesh* quad;
	Mesh* tower;
	Mesh* tree;

	Light* light;
	Camera* camera;
	SceneNode* treeRoot;

	GLuint cubeMap;
	GLuint earthTex;
	GLuint earthBump;
	GLuint forestTex;
	GLuint forestBump;
	GLuint coastTex;
	GLuint coastBump;
	GLuint snowTex;
	GLuint snowBump;
	GLuint towerTex;
	GLuint towerBump;
	GLuint mainTreeTex;

	Frustum frameFrustum;

	unsigned int uboMatrix;
	vector<SceneNode*> nodeList;

};
