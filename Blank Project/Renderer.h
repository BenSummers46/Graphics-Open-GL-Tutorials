#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "../nclgl/Frustum.h"

class Camera;
class Shader;
class HeightMap;
class MeshAnimation;

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
	void DrawAnimation();
	void GenerateBuffers();
	void DrawPostProcess();

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
	Shader* animShader;
	Shader* processShader;

	HeightMap* heightMap;
	HeightMap* waterMap;
	Mesh* quad;
	Mesh* tower;
	Mesh* tree;
	Mesh* animMesh;
	MeshAnimation* animation;

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
	GLuint animTex;

	Frustum frameFrustum;
	GLuint bufferFBO;
	GLuint processFBO;
	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;

	unsigned int uboMatrix;
	vector<SceneNode*> nodeList;

	int currentFrame;
	float frameTime;
	float spiderMove;
};
