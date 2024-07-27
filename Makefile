# To build run: make
# To delete all binaries run: make clean
# To build & run progam run: make run

CXX := g++
OUTPUT := sfmlgame

CXX_FLAGS := -o3 -std=c++17
LDFLAGS := -o3 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

all : game.exe

game.exe : main.o Action.o Animation.o Assets.o Entity.o EntityManager.o GameEngine.o Physics.o Scene_Menu.o Scene_Play.o Scene.o Vec2.o
	$(CXX) $(CXX_FLAGS) ./bin/main.o ./bin/Action.o ./bin/Animation.o ./bin/Assets.o ./bin/Entity.o ./bin/EntityManager.o ./bin/GameEngine.o ./bin/Physics.o ./bin/Scene_Menu.o ./bin/Scene_Play.o ./bin/Scene.o ./bin/Vec2.o $(LDFLAGS) -o ./bin/game.exe 

main.o : ./src/main.cpp
	$(CXX) $(CXX_FLAGS) -c ./src/main.cpp -o ./bin/main.o

Action.o : ./src/Action.cpp ./src/Action.h
	$(CXX) $(CXX_FLAGS) -c ./src/Action.cpp -o ./bin/Action.o

Animation.o : ./src/Animation.cpp ./src/Animation.h
	$(CXX) $(CXX_FLAGS) -c ./src/Animation.cpp -o ./bin/Animation.o

Assets.o : ./src/Assets.cpp ./src/Assets.h
	$(CXX) $(CXX_FLAGS) -c ./src/Assets.cpp -o ./bin/Assets.o

Entity.o : ./src/Entity.cpp ./src/Entity.h
	$(CXX) $(CXX_FLAGS) -c ./src/Entity.cpp -o ./bin/Entity.o

EntityManager.o : ./src/EntityManager.cpp ./src/EntityManager.h
	$(CXX) $(CXX_FLAGS) -c ./src/EntityManager.cpp -o ./bin/EntityManager.o

GameEngine.o : ./src/GameEngine.cpp ./src/GameEngine.h
	$(CXX) $(CXX_FLAGS) -c ./src/GameEngine.cpp -o ./bin/GameEngine.o

Physics.o : ./src/Physics.cpp ./src/Physics.h
	$(CXX) $(CXX_FLAGS) -c ./src/Physics.cpp -o ./bin/Physics.o

Scene_Menu.o : ./src/Scene_Menu.cpp ./src/Scene_Menu.h
	$(CXX) $(CXX_FLAGS) -c ./src/Scene_Menu.cpp -o ./bin/Scene_Menu.o

Scene_Play.o : ./src/Scene_Play.cpp ./src/Scene_Play.h
	$(CXX) $(CXX_FLAGS) -c ./src/Scene_Play.cpp -o ./bin/Scene_Play.o

Scene.o : ./src/Scene.cpp ./src/Scene.h
	$(CXX) $(CXX_FLAGS) -c ./src/Scene.cpp -o ./bin/Scene.o

Vec2.o : ./src/Vec2.cpp ./src/Vec2.h
	$(CXX) $(CXX_FLAGS) -c ./src/Vec2.cpp -o ./bin/Vec2.o

clean : 
	rm -f ./bin/*.o ./bin/*.exe