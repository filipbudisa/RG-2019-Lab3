#include "Game.h"
#include "storage/Storage.h"
#include <optional>

Player *staticPlayer;
Game *staticGame;
double oldX = -1, oldY = -1;
bool drawMesh = false;

void staticKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods){
	bool on;

	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, true);
	}

	if(action == GLFW_PRESS){
		on = true;
	}else if(action == GLFW_RELEASE){
		on = false;
	}else{
		return;
	}

	int code = 0;

	switch(key){
		case GLFW_KEY_W:
			code = 1;
			break;
		case GLFW_KEY_S:
			code = 2;
			break;
		case GLFW_KEY_A:
			code = 3;
			break;
		case GLFW_KEY_D:
			code = 4;
			break;
		case GLFW_KEY_SPACE:
			code = 5;
			break;
		case GLFW_KEY_X:
			code = 6;
			break;
		case GLFW_KEY_1:
			if(on) staticGame->loadScene(1);
			break;
		case GLFW_KEY_2:
			if(on) staticGame->loadScene(2);
			break;
		case GLFW_KEY_F:
			if(on) Storage::worldObjects[1]->renderComponent->pipeline = 2 * !Storage::worldObjects[1]->renderComponent->pipeline;
			break;
		case GLFW_KEY_G:
			if(on) drawMesh = !drawMesh;
			break;
	}

	if(code != 0){
		staticPlayer->input(code, on);
	}
}

void staticCursorPosCallback(GLFWwindow *window, double xpos, double ypos){
	if(oldX == -1 && oldY == -1){
		oldX = xpos;
		oldY = ypos;
		return;
	}

	staticPlayer->cursor(xpos - oldX, ypos - oldY);
	oldX = xpos;
	oldY = ypos;
}

void Game::init(int scene){
	staticGame = this;

	graphics = new Graphics();
	graphics->init();

	physics = new PhysicsEngine();

	Storage::init(graphics);
	world = new World();
	loadScene(scene);

	graphics->initCompute();

	player = new Player(graphics->getCamera());
	staticPlayer = player;

	//glfwSetInputMode(graphics->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	/*if(glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);*/

	glfwSetKeyCallback(graphics->window, staticKeyCallback);
	glfwSetCursorPosCallback(graphics->window, staticCursorPosCallback);
}

void Game::run(){
	while(!glfwWindowShouldClose(graphics->window)){

		glfwPollEvents();

		updateLogic();

		graphics->drawFrame();
	}

	graphics->wait();
	world->cleanup();
	graphics->cleanup();
}

double t = 0;
int t2 = 0;

void Game::updateLogic(){
	auto timeNow = Clock::now();
	double elapsed = std::chrono::duration<float, std::chrono::seconds::period>(timeNow - time).count();

	if(time.time_since_epoch().count() == 0){
		time = timeNow;
		return;
	}else{
		time = timeNow;
	}

	t += elapsed;
	if(((int) t) != t2){
		t2 = (int) t;
		printf("Time: %ds\n", t2);
	}

	world->update(elapsed);
	physics->update(elapsed, graphics);
	player->update(elapsed);
}

void Game::loadScene(int i){
	graphics->wait();
	world->cleanup();
	graphics->clear();

	world->load(i);

	graphics->initData();
}
