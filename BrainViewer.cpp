using namespace std;
#include "organism.h"
#include "HexSim.h"
#include "GlutStuff.h"

int main (int argc, char *argv[])
{
	HexSim sim;
	organism *bob; // always bob
	std::mt19937 eng((std::random_device())());

	sim.initPhysics();

	if (argc == 1)
	{
		cout << "Using random brain. Enjoy." << endl;
		bob = new organism(MUTATION_NEW, &eng);
	} else if (argc == 2) {
		cout << "Loading brain from file." << endl;
		bob = new organism(0.0, &eng);
		bob->readFromFile(argv[1]);
	} else {
		cout << "Invalid number of command-line arguments!" << endl;
		return -1;
	}

	sim.attachOrganism(bob);
	
	return glutmain(argc, argv, 1280, 960, "BrainViewer", &sim);
}
