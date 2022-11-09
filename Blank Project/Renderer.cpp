#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/Camera.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{
	quad = Mesh::GenerateQuad();

	heightMap = new HeightMap(TEXTUREDIR"Terrain_heightmap8.png");
	
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR "rocks.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR "rocks_normal.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg", TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg", TEXTUREDIR"rusted_south.jpg",
		TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	forestTex = SOIL_load_OGL_texture(TEXTUREDIR"forrest_ground.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	forestBump = SOIL_load_OGL_texture(TEXTUREDIR"forrest_ground_bump.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	coastTex = SOIL_load_OGL_texture(TEXTUREDIR"coast_sand.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	coastBump = SOIL_load_OGL_texture(TEXTUREDIR"coast_sand_bump.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	snowTex = SOIL_load_OGL_texture(TEXTUREDIR"snow2.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	snowBump = SOIL_load_OGL_texture(TEXTUREDIR"snow_bump2.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	if (!earthTex || !earthBump || !cubeMap || !forestTex || !forestBump || !coastTex || !coastBump) {
		return;
	}

	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(forestTex, true);
	SetTextureRepeating(forestBump, true);
	SetTextureRepeating(coastTex, true);
	SetTextureRepeating(coastBump, true);
	SetTextureRepeating(snowTex, true);
	SetTextureRepeating(snowBump, true);

	skyboxShader = new Shader("SkyboxVertex.glsl", "SkyboxFragment.glsl");
	lightShader = new Shader("coursework/BumpBlendVertex.glsl", "coursework/BumpBlendFrag.glsl");
	

	if (!skyboxShader->LoadSuccess() || !lightShader->LoadSuccess()) {
		return;
	}

	Vector3 heightmapSize = heightMap->GetHeightMapSize();

	camera = new Camera(-45.0f, 0.0f, heightmapSize * Vector3(0.5f, 1.0f, 0.5f)); //was 5
	light = new Light(heightmapSize * Vector3(1.0f, 1.0f, 0.5f), Vector4(1, 1, 1, 1), Vector4(1, 1, 1, 1), heightmapSize.x * 4); //was 20

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	init = true;
}
Renderer::~Renderer(void)	{
	delete camera;
	delete heightMap;
	delete quad;
	delete skyboxShader;
	delete lightShader;
	delete light;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
}

void Renderer::RenderScene()	{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkyBox();
	DrawHeightMap();
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

