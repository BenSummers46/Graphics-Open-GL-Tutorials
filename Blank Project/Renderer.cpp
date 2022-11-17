#include "Renderer.h"
#include <iostream>
#include "../nclgl/Light.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/Camera.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/MeshAnimation.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{
	quad = Mesh::GenerateQuad();
	tower = Mesh::LoadFromMeshFile("RuinedTower.msh");
	tree = Mesh::LoadFromMeshFile("Cedar03.msh");

	heightMap = new HeightMap(TEXTUREDIR"Terrain_heightmap8.png");
	waterMap = new HeightMap(TEXTUREDIR"WaterTerrain_heightmap8.png");
	
	animMesh = Mesh::LoadFromMeshFile("Spider.msh");
	animation = new MeshAnimation("Spider.anm");

	LoadTextures();

	if (!earthTex || !earthBump || !cubeMap || !forestTex || !forestBump || !coastTex || !coastBump || !towerTex || !towerBump || !mainTreeTex || !animTex) {
		return;
	}

	SetTextureRepeating(earthTex, true); SetTextureRepeating(earthBump, true);
	SetTextureRepeating(forestTex, true); SetTextureRepeating(forestBump, true);
	SetTextureRepeating(coastTex, true); SetTextureRepeating(coastBump, true);
	SetTextureRepeating(snowTex, true); SetTextureRepeating(snowBump, true);
	SetTextureRepeating(mainTreeTex, true);

	skyboxShader = new Shader("SkyboxVertex.glsl", "SkyboxFragment.glsl");
	lightShader = new Shader("coursework/BumpBlendVertex.glsl", "coursework/BumpBlendFrag.glsl");
	waterShader = new Shader("coursework/WaterVertex.glsl", "coursework/WaterFragment.glsl");
	towerShader = new Shader("coursework/TowerVertex.glsl", "coursework/TowerFragment.glsl");
	mainTreeShader = new Shader("coursework/TreeVertex.glsl", "coursework/TreeFragment.glsl");
	forestShader = new Shader("coursework/ForestVertex.glsl", "coursework/ForestFragment.glsl");
	animShader = new Shader("coursework/AnimationVertex.glsl", "coursework/TexturedWolf.glsl");
	processShader = new Shader("TexturedVertex.glsl", "ProcessFrag.glsl");
	sceneShader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");

	if (!skyboxShader->LoadSuccess() || !lightShader->LoadSuccess() || !waterShader->LoadSuccess() || !towerShader->LoadSuccess() || !mainTreeShader->LoadSuccess() ||
		!forestShader->LoadSuccess() || !animShader->LoadSuccess() || !processShader->LoadSuccess() || !sceneShader->LoadSuccess()) {
		return;
	}

	Vector3 heightmapSize = heightMap->GetHeightMapSize();

	camera = new Camera(-45.0f, 0.0f, heightmapSize * Vector3(0.5f, 1.0f, 0.5f)); 
	light = new Light(heightmapSize * Vector3(0.4f, 1.3f, 1.0f), Vector4(1, 1, 1, 1), Vector4(1, 1, 1, 1), heightmapSize.x * 2); 

	treeRoot = new SceneNode();
	treeRoot->SetTransform(Matrix4::Translation(Vector3(heightmapSize.x * 0.75, heightmapSize.y * 0.06, heightmapSize.z * 0.35)));

	for (int i = 0; i < 50; ++i) {
		SceneNode* s = new SceneNode();
		s->SetColour(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		s->SetTransform(Matrix4::Translation(Vector3(rand() % 1400 - (rand() % 300), 1, rand() % 2600 - (rand() % 200))));
		s->SetModelScale(Vector3(20.0f, (rand() % 25) + 15, 20.0f)); 
		s->SetBoundingRadius(500.0f);
		s->SetMesh(tree);
		s->SetTexture(mainTreeTex);
		treeRoot->AddChild(s);
	}

	projMatrix = Matrix4::Perspective(1.0f, 20000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	sceneTime = 0.0f;
	currentFrame = 0;
	frameTime = 0.0f;
	spiderMove = 0.65f;
	POST_PASSES = 0;
	CreateMatrixUBO();
	GenerateBuffers();

	init = true;
}

Renderer::~Renderer(void)	{
	delete camera;
	delete heightMap;
	delete quad;
	delete tower;
	delete tree;
	delete skyboxShader;
	delete lightShader;
	delete waterShader;
	delete towerShader;
	delete mainTreeShader;
	delete forestShader;
	delete processShader;
	delete sceneShader;
	delete animShader;
	delete light;
	delete treeRoot;
	delete animMesh;
	delete animation;

	glDeleteTextures(2, bufferColourTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &processFBO);
}

void Renderer::GenerateBuffers() {
	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	for (int i = 0; i < 2; ++i) {
		glGenTextures(1, &bufferColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
	}

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &processFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferColourTex[0]) {
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::LoadTextures() {
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR "rocks1k.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR "rocks1knormal.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"Space_left.png", TEXTUREDIR"Space_right.png", TEXTUREDIR"Space_up.png", TEXTUREDIR"Space_down.png", TEXTUREDIR"Space_front.png",
		TEXTUREDIR"Space_back.png", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	forestTex = SOIL_load_OGL_texture(TEXTUREDIR"forest1k.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	forestBump = SOIL_load_OGL_texture(TEXTUREDIR"forest1knormal.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	coastTex = SOIL_load_OGL_texture(TEXTUREDIR"coast_sand.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	coastBump = SOIL_load_OGL_texture(TEXTUREDIR"coast_sand_bump.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	snowTex = SOIL_load_OGL_texture(TEXTUREDIR"snow1k.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	snowBump = SOIL_load_OGL_texture(TEXTUREDIR"snow1knormal.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	towerTex = SOIL_load_OGL_texture(TEXTUREDIR"RuinedTower_vcols.bmp", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	towerBump = SOIL_load_OGL_texture(TEXTUREDIR"RuinedTower_normals.bmp", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	mainTreeTex = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	animTex = SOIL_load_OGL_texture(TEXTUREDIR"Spider.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 20000.0f, (float)width / (float)height, 45.0f);
	treeRoot->Update(dt);

	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrix);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Matrix4), sizeof(Matrix4), viewMatrix.values) ;
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % animation->GetFrameCount();
		frameTime += 1.0f / animation->GetFrameRate();
	}
	
	BindShader(waterShader);
	sceneTime += dt;
	glUniform1f(glGetUniformLocation(waterShader->GetProgram(), "time"), sceneTime);
}

void Renderer::RenderScene()	{
	DrawScene();

	DrawPostProcess();
	PresentScene();
}

void Renderer::ToggleBlur() {
	if (POST_PASSES == 0) {
		POST_PASSES = 10;
	}
	else {
		POST_PASSES = 0;
	}
}

void Renderer::DrawScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	DrawSkyBox();
	DrawHeightMap();
	DrawWater();
	DrawTower();
	DrawTree();
	DrawForest();
	DrawAnimation();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawPostProcess() {
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(processShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(processShader->GetProgram(), "sceneTex"), 0);

	for (int i = 0; i < POST_PASSES; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 0);

		glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
		quad->Draw();

		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[1]);
		quad->Draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::PresentScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(sceneShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	quad->Draw();
}

void Renderer::DrawAnimation() {
	BindShader(animShader);
	Vector3 hSize = heightMap->GetHeightMapSize();

	modelMatrix = Matrix4::Translation(Vector3(hSize.x * spiderMove, hSize.y * 0.055f, hSize.z * 0.68f)) * Matrix4::Scale(Vector3(100, 100, 100)) * Matrix4::Rotation(95, Vector3(0, 1, 0));
	glUniform1i(glGetUniformLocation(animShader->GetProgram(), "diffuseTex"), 0);
	UpdateShaderMatrices();

	vector<Matrix4> frameMatrices;
	const Matrix4* invBindPose = animMesh->GetInverseBindPose();
	const Matrix4* frameData = animation->GetJointData(currentFrame);

	for (unsigned int i = 0; i < animMesh->GetJointCount(); ++i) {
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(animShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	for (int i = 0; i < animMesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, animTex);

		animMesh->DrawSubMesh(i);
	}
	if (spiderMove <= 0.0f) {
		spiderMove += 0.0001f;
	}
	else {
		spiderMove -= 0.0001f;
	}
}

void Renderer::DrawForest() {
	BuildNodeLists(treeRoot);
	SortNodeLists();
	BindShader(forestShader);
	UpdateShaderMatrices();
	DrawNodes();
	ClearNodeLists();
}

void Renderer::ClearNodeLists() {
	nodeList.clear();
}

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		glUniformMatrix4fv(glGetUniformLocation(forestShader->GetProgram(), "modelMatrix"), 1, false, model.values);

		glUniform1i(glGetUniformLocation(forestShader->GetProgram(), "diffuseTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, n->GetTextures());

		n->Draw(*this);
	}
}

void Renderer::DrawNodes() {
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
}

void Renderer::SortNodeLists() {
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
}

void Renderer::BuildNodeLists(SceneNode* from) {
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));
		nodeList.push_back(from);
	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i) {
		BuildNodeLists(*i);
	}
}

void Renderer::DrawTree() {
	BindShader(mainTreeShader);

	glUniform1i(glGetUniformLocation(mainTreeShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mainTreeTex);

	Vector3 hSize = heightMap->GetHeightMapSize();

	modelMatrix = Matrix4::Translation(Vector3(hSize.x * 2.0, hSize.y * 0.5f, hSize.z * 0.48f)) * Matrix4::Scale(Vector3(hSize.x * 0.07f, hSize.y * 0.3f, hSize.z * 0.07f)) * Matrix4::Rotation(0, Vector3(0, 0, 0));
	UpdateShaderMatrices();

	tree->Draw();
}

void Renderer::DrawTower() {
	BindShader(towerShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(towerShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	
	glUniform1i(glGetUniformLocation(towerShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, towerTex);

	glUniform1i(glGetUniformLocation(towerShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, towerBump);

	Vector3 hSize = heightMap->GetHeightMapSize();

	modelMatrix = Matrix4::Translation(Vector3(hSize.x * 0.77f, hSize.y * 0.04f, hSize.z * 0.57f)) * Matrix4::Scale(Vector3(hSize.x * 0.009f, hSize.y * 0.02f, hSize.z * 0.009f)) * Matrix4::Rotation(85, Vector3(0, 1, 0)); 

	UpdateShaderMatrices();
	tower->Draw();
}

void Renderer::DrawWater() {
	BindShader(waterShader);

	glGetUniformBlockIndex(waterShader->GetProgram(), "Uniforms");
	
	glUniform3fv(glGetUniformLocation(waterShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	
	glUniform1i(glGetUniformLocation(waterShader->GetProgram(), "cubeTex"), 8);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
	

	Vector3 hSize = heightMap->GetHeightMapSize();
	
	modelMatrix = Matrix4::Translation(Vector3(hSize.x * 0.0f, hSize.y * 0.035f, hSize.z * 0.0f)) * Matrix4::Scale(hSize * 0.0001f) * Matrix4::Rotation(0, Vector3(1, 0, 0));

	UpdateShaderMatrices();
	waterMap->Draw();
}

void Renderer::DrawSkyBox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightMap() {
	BindShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1f(glGetUniformLocation(lightShader->GetProgram(), "terrainHeight"), heightMap->GetHeightMapSize().y);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "forestTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, forestTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "forestBump"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, forestBump);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "coastTex"), 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, coastTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "coastBump"), 5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, coastBump);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "snowTex"), 6);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, snowTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "snowBump"), 7);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, snowBump);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	heightMap->Draw();
}

void Renderer::CreateMatrixUBO() {
	glGenBuffers(1, &uboMatrix);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrix);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(Matrix4), projMatrix.values, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	unsigned int block_index = glGetUniformBlockIndex(mainTreeShader->GetProgram(), "matrices");
	unsigned int water_index = glGetUniformBlockIndex(waterShader->GetProgram(), "matrices");
	unsigned int land_index = glGetUniformBlockIndex(lightShader->GetProgram(), "matrices");
	unsigned int forest_index = glGetUniformBlockIndex(forestShader->GetProgram(), "matrices");
	
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboMatrix);
	glUniformBlockBinding(mainTreeShader->GetProgram(), block_index, 1);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboMatrix);
	glUniformBlockBinding(waterShader->GetProgram(), water_index, 2);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, uboMatrix);
	glUniformBlockBinding(lightShader->GetProgram(), land_index, 3);
	glBindBufferBase(GL_UNIFORM_BUFFER, 4, uboMatrix);
	glUniformBlockBinding(forestShader->GetProgram(), forest_index, 4);
}

