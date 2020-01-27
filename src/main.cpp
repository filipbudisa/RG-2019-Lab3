#include "Game.h"
#include "data.h"

int noPoints = 10;

int main(int argc, char** argv){
	Game game;

	int scene = 1;
	if(argc >= 2){
		scene = atoi(argv[1]);
	}

	if(argc >= 3){
		noPoints = atoi(argv[2]);
	}

	srand(time(0));

	try{
		game.init(scene);
		game.run();
	}catch(const std::exception &e){
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
