using namespace std;

#include "organism.h"
#include "HexSim.h"
#include "GlutStuff.h"

int main(int argc,char* argv[])
{
	int ii;
  HexSim sim;
	organism bob(true);

  sim.initPhysics();
	sim.attachOrganism(&bob);

	// can do roughly 2500 steps per second
	// or about 25x real-time
	for (ii=0; ii<1000; ii++)
		if (!sim.converged)
			sim.stepSimulation(0.01, 1);

	cout << sim.getFitness() << endl;
  return 0;
	return glutmain(argc, argv, 1280, 960, "HexSim", &sim);
}
