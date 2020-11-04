
INCLUDE = -I ./include
OBJ =  ./obj/shader.o ./obj/glad.o ./obj/mesh.o ./obj/animator.o ./obj/model.o ./obj/main.o
DIST = -g
main: $(OBJ)
	g++ $(DIST) -I ./include $(OBJ) -lglfw -ldl -lassimp

./obj/main.o: ./src/main.cpp
	g++ $(DIST) $(INCLUDE) ./src/main.cpp  -c -o ./obj/main.o

./obj/shader.o: ./src/shader.cpp
	g++ $(DIST) $(INCLUDE) ./src/shader.cpp -c -o ./obj/shader.o

./obj/glad.o: ./src/glad/glad.c
	gcc $(DIST) $(INCLUDE) ../src/glad/glad.c -c -o ./obj/glad.o

./obj/model.o: ./src/model.cpp
	g++ $(DIST) $(INCLUDE) ./src/model.cpp -c -o ./obj/model.o

./obj/mesh.o: ./src/mesh.cpp
	g++ $(DIST) $(INCLUDE) ./src/mesh.cpp -c -o ./obj/mesh.o

./obj/animator.o: ./src/animator.cpp
	g++ $(DIST) $(INCLUDE) ./src/animator.cpp  -c -o ./obj/animator.o
