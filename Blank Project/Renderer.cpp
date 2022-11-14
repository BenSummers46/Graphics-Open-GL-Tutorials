#include "Renderer.h"
#include <iostream>
#include "../nclgl/Light.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/Camera.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{
	quad = Mesh::GenerateQuad();
	tower = Mesh::LoadFromMeshFile("RuinedTower.msh");
	tree = Mesh::LoadFromMeshFile("Cedar03.msh");

	heightMap = new HeightMap(TEXTUREDIR"Terrain_heightmap8.png");
	waterMap = new HeightMap(TEXTUREDIR"WaterTerrain_heightmap8.png");

	LoadTextures();

	if (!earthTex || !earthBump || !cubeMap || !forestTex || !forestBump || !coastTex || !coastBump || !towerTex || !towerBump || !mainTreeTex) {
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
	

	if (!skyboxShader->LoadSuccess() || !lightShader->LoadSuccess() || !waterShader->LoadSuccess() || !towerShader->LoadSuccess() || !mainTreeShader->LoadSuccess()) {
		return;
	}

	Vector3 heightmapSize = heightMap->GetHeightMapSize();

	camera = new Camera(-45.0f, 0.0f, heightmapSize * Vector3(0.5f, 1.0f, 0.5f)); //was 5
	light = new Light(heightmapSize * Vector3(1.0f, 1.5f, 1.0f), Vector4(1, 1, 1, 1), Vector4(1, 1, 1, 1), heightmapSize.x * 2); //was 20

	projMatrix = Matrix4::Perspective(1.0f, 20000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	sceneTime = 0.0f;

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
	delete light;
}

void Renderer::LoadTextures() {
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR "rocks.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR "rocks_normal.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"Space_left.png", TEXTUREDIR"Space_right.png", TEXTUREDIR"Space_up.png", TEXTUREDIR"Space_down.png", TEXTUREDIR"Space_front.png",
		TEXTUREDIR"Space_back.png", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	forestTex = SOIL_load_OGL_texture(TEXTUREDIR"forrest_ground.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	forestBump = SOIL_load_OGL_texture(TEXTUREDIR"forrest_ground_bump.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	coastTex = SOIL_load_OGL_texture(TEXTUREDIR"coast_sand.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	coastBump = SOIL_load_OGL_texture(TEXTUREDIR"coast_sand_bump.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	snowTex = SOIL_load_OGL_texture(TEXTUREDIR"snow2.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	snowBump = SOIL_load_OGL_texture(TEXTUREDIR"snow_bump2.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	towerTex = SOIL_load_OGL_texture(TEXTUREDIR"RuinedTower_vcols.bmp", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	towerBump = SOIL_load_OGL_texture(TEXTUREDIR"RuinedTower_normals.bmp", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	mainTreeTex = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	
	BindShader(waterShader);
	sceneTime += dt;
	glUniform1f(glGetUniformLocation(waterShader->GetProgram(), "time"), sceneTime);
}

void Renderer::RenderScene()	{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkyBox();
	DrawHeightMap();
	DrawWater();
	DrawTower();
	DrawTree();
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

	modelMatrix = Matrix4::Translation(Vector3(hSize.x * 0.85f, hSize.y * 0.05f, hSize.z * 0.48f)) * Matrix4::Scale(Vector3(hSize.x * 0.007f, hSize.y * 0.012f, hSize.z * 0.007f)) * Matrix4::Rotation(125, Vector3(0, 1, 0));

	UpdateShaderMatrices();
	tower->Draw();
}

void Renderer::DrawWater() {
	BindShader(waterShader);

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

