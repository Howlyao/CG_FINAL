#include "Game.h"


Scene *scene = NULL;
ParticleSystem *particleSystem = NULL;
Player *player = NULL;
Sea *sea = NULL;

Game::Game(int width,int height) {
	this->Width = width;
	this->Height = height;

	//initial Key to false
	memset(Keys, false, sizeof(Keys));
	memset(KeysProcessed, false, sizeof(KeysProcessed));
	
}

Game::~Game() {
	delete scene;
	delete particleSystem;
	delete player;
	delete sea;

}

void Game::Init() {
	shader = Shader("resources/shaders/depthShader/test.vs", "resources/shaders/depthShader/test.fs");

	this->State = GAME_ACTIVE;
	// load model
	ResourceManager::LoadModel("resources/models/scene/scene.obj", "scene");
	ResourceManager::LoadModel("resources/models/snowman/snowman.obj", "snowman");
	ResourceManager::LoadModel("resources/models/QQ/qq.obj", "qq");
	ResourceManager::LoadModel("resources/models/ice/ice.obj", "ice");
	
	unsigned int depthMap = ResourceManager::CreateFrameBuffer("depthFBO", Width, Height);
	ResourceManager::LoadTexture(depthMap, "depthMap");
	// load shader
	Shader depthShader = ResourceManager::LoadShader("resources/shaders/depthShader/depthShader.vs", "resources/shaders/depthShader/depthShader.fs", NULL, "depthShader");
	Shader lightShader = ResourceManager::LoadShader("resources/shaders/lightShader/lightShader.vs", "resources/shaders/lightShader/lightShader.fs", NULL, "lightShader");
	Shader snowShader = ResourceManager::LoadShader("resources/shaders/snow/snow.vs", "resources/shaders/snow/snow.fs", NULL, "snowShader");
	Shader snowmanShader = ResourceManager::LoadShader("resources/shaders/lightShader/lightShader.vs", "resources/shaders/snowman/snowman.fs", NULL, "snowmanShader");
	// get shader
	
	//load Texture
	unsigned int snowTexture = ResourceManager::LoadTexture("resources/textures/particles/snow.png", "snow");
	
	// Set Shader Texture
	lightShader.use();
	lightShader.setInt("depthMap", 6);
	snowShader.use();
	snowShader.setInt("snowTexture", 0);
	
	// set light space matrix
	float near_plane = 2.0f, far_plane = 100.0f;
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	depthShader.use();
	depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	lightShader.use();
	lightShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	lightShader.setVec3("dirLight.direction", lightDirection);
	lightShader.setVec3("dirLight.color", lightColor);
	lightShader.setVec3("dirLight.ambient", glm::vec3(0.5f, 0.5f, 0.5f));
	lightShader.setVec3("dirLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
	lightShader.setVec3("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));
	snowmanShader.use();
	snowmanShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	snowmanShader.setVec3("dirLight.direction", lightDirection);
	snowmanShader.setVec3("dirLight.color", lightColor);
	snowmanShader.setVec3("dirLight.ambient", lightAmbient);
	snowmanShader.setVec3("dirLight.diffuse", lightDiffuse);
	snowmanShader.setVec3("dirLight.specular", lightSpecular);

	//
	scene = new Scene();
	scene->createIces(20);
	//particle
	particleSystem = new ParticleSystem(100, glm::vec3(0.0f, 0.0f, 0.0f), 20.0f, 20.0f, 10.0f);

	//player
	player = new Player(playerPos, playerSize, 0.0f, "snowman");
	glm::vec3 Pos = player->position;
	camera.Position = Pos + glm::vec3(0.0f, 6.0f, 0.0f);

	//Sea
	sea = new Sea();
}	

void Game::ProcessInput(float dt) {
	if (this->State == GAME_ACTIVE) {
		if (this->Keys[GLFW_KEY_ESCAPE]) {
			this->State = GAME_PAUSE;
			//show menu
			
		}
		if (this->Keys[GLFW_KEY_W]) {
			player->Move(FORWARD, dt);
		}
		if (this->Keys[GLFW_KEY_S]) {
			player->Move(BACKWARD, dt);
		}
		if (this->Keys[GLFW_KEY_A]) {
			player->Move(LEFT, dt);
		}
		if (this->Keys[GLFW_KEY_D]) {
			player->Move(RIGHT, dt);
		}
		camera.Position = player->position;




	}
	else if (this->State == GAME_PAUSE) {
		if (this->Keys[GLFW_KEY_ESCAPE]) {
			this->State = GAME_ACTIVE;
		}
	}
}

void Game::ProcessMouseMovement(float offset) {
	player->Rotate();
	
}


void Game::Update(float dt) {
	if (this->State == GAME_ACTIVE) {
		particleSystem->Update(dt);
		DoCollisions();
		scene->Update();
		player->Update(dt);
	}
}

void Game::Render() {
	if (this->State == GAME_ACTIVE) {
		//Shadow mapping
		//renderScene(true);
		//render scene width shadow mapping
		renderScene(false);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glViewport(0, 0, Width, Height);
		// //clear all relevant buffers
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//renderQuad();
		
		
	}
	if (this->State == GAME_PAUSE) {

	}
}

void Game::renderScene(bool isShadow) {
	//��Ӱ��ͼ
	if (isShadow) {

		unsigned int depthFBO = ResourceManager::GetFrameBuffer("depthFBO");
		Shader depthShader = ResourceManager::GetShader("depthShader");
		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
		glViewport(0, 0, Width, Height);
		glClear(GL_DEPTH_BUFFER_BIT);
		
		depthShader.use();
		renderScene0(depthShader);
	
		scene->Draw(depthShader, true);
		player->Draw(depthShader, true);
		
	}
	else {
		// Load shadow map texture
		unsigned int depthMap = ResourceManager::GetTexture("depthMap");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		// Load shader and set uniform variable
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)Width / (float)Height, 0.1f, 1000.0f);
		Shader lightShader = ResourceManager::GetShader("lightShader");
		scene->Draw(lightShader, false);

		Shader snowShader = ResourceManager::GetShader("snowShader");
		particleSystem->Draw(snowShader);

		//Player render
		Shader snowmanShader = ResourceManager::GetShader("snowmanShader");
		player->Draw(snowmanShader, false);

		sea->Draw();
	}
}

void Game::DoCollisions() {
	for (GameObject &box : scene->ices)
	{
		if (!box.destroyed)
		{
			if (CheckCollision(*player, box))
			{
				player->snowmanHeight += 0.05f;
				box.destroyed = true;
			}
		}
	}
}

bool Game::CheckCollision(GameObject &one, GameObject &two) {
	bool collisionX = one.position.x + one.size.x / 2 >= two.position.x - two.size.x / 2 &&
		two.position.x + two.size.x / 2 >= one.position.x - one.size.x / 2;
	
	bool collisionY = one.position.y + one.size.y / 2 >= two.position.y - two.size.y / 2 &&
		two.position.y + two.size.y / 2>= one.position.y - one.size.y / 2;

	bool collisionZ = one.position.z + one.size.z / 2 >= two.position.z - two.size.z / 2 &&
		two.position.z + two.size.z / 2 >= one.position.z - one.size.z / 2;
	
	return collisionX && collisionY && collisionZ;
}