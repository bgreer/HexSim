
FLAGS = -std=c++11 -O2

HexSim : HexSim.cpp HexSim.h main.cpp organism.h tuning.h TestRig.h
	c++ $(FLAGS) main.cpp HexSim.cpp -o HexSim -rdynamic /home/bgreer/SOFTWARE/bullet-2.82-r2704/Demos/OpenGL/libOpenGLSupport.so ./bullet/BulletDynamics/libBulletDynamics.so ./bullet/BulletCollision/libBulletCollision.so ./bullet/LinearMath/libLinearMath.so -lglut -lGL -lGLU -Wl,-rpath,/home/bgreer/SOFTWARE/bullet-2.82-r2704/Demos/OpenGL:/home/bgreer/SOFTWARE/bullet-2.82-r2704/src/BulletDynamics:/home/bgreer/SOFTWARE/bullet-2.82-r2704/src/BulletCollision:/home/bgreer/SOFTWARE/bullet-2.82-r2704/src/LinearMath -I./bullet -I/home/bgreer/SOFTWARE/bullet-2.82-r2704/Demos/OpenGL/

clean :
	rm -f HexSim
