
FLAGS = -std=c++11 -O2

all : HexSim BrainViewer

HexSim : HexSim.cpp HexSim.h main.cpp organism.h tuning.h TestRig.h servo.h
	c++ $(FLAGS) main.cpp HexSim.cpp -o HexSim -rdynamic /home/bgreer/SOFTWARE/bullet-2.82-r2704/Demos/OpenGL/libOpenGLSupport.so ./bullet/BulletDynamics/libBulletDynamics.so ./bullet/BulletCollision/libBulletCollision.so ./bullet/LinearMath/libLinearMath.so -lglut -lGL -lGLU -Wl,-rpath,/home/bgreer/SOFTWARE/bullet-2.82-r2704/Demos/OpenGL:/home/bgreer/SOFTWARE/bullet-2.82-r2704/src/BulletDynamics:/home/bgreer/SOFTWARE/bullet-2.82-r2704/src/BulletCollision:/home/bgreer/SOFTWARE/bullet-2.82-r2704/src/LinearMath -I./bullet -I/home/bgreer/SOFTWARE/bullet-2.82-r2704/Demos/OpenGL/

BrainViewer : BrainViewer.cpp HexSim.h HexSim.cpp organism.h tuning.h TestRig.h servo.h
	c++ $(FLAGS) BrainViewer.cpp HexSim.cpp -o BrainViewer -rdynamic /home/bgreer/SOFTWARE/bullet-2.82-r2704/Demos/OpenGL/libOpenGLSupport.so ./bullet/BulletDynamics/libBulletDynamics.so ./bullet/BulletCollision/libBulletCollision.so ./bullet/LinearMath/libLinearMath.so -lglut -lGL -lGLU -Wl,-rpath,/home/bgreer/SOFTWARE/bullet-2.82-r2704/Demos/OpenGL:/home/bgreer/SOFTWARE/bullet-2.82-r2704/src/BulletDynamics:/home/bgreer/SOFTWARE/bullet-2.82-r2704/src/BulletCollision:/home/bgreer/SOFTWARE/bullet-2.82-r2704/src/LinearMath -I./bullet -I/home/bgreer/SOFTWARE/bullet-2.82-r2704/Demos/OpenGL/


clean :
	rm -f HexSim brain_viewer
