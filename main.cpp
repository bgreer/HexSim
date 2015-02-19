using namespace std;
#include <algorithm>
#include "organism.h"
#include "HexSim.h"
#include "GlutStuff.h"

bool compareOrganisms (organism *a, organism *b) {return (a->fitness>b->fitness);}

int main(int argc,char* argv[])
{
	int ii, ij, ik, parentA, parentB;
	string fname;
  HexSim sim;
	vector<organism*> gen, nextgen;
	organism *bob;

  sim.initPhysics();
	std::mt19937 eng((std::random_device())());
	std::exponential_distribution<float> dist_exp(3.0/GEN_SIZE);

	// populate first generation
	for (ii=0; ii<GEN_SIZE; ii++)
	{
		gen.push_back(new organism(true));
	}
	nextgen.resize(GEN_SIZE);

	// iterate generations
	for (ij=0; ij<NUM_GENS; ij++)
	{
		// march through each organism in the current generation
		for (ii=0; ii<GEN_SIZE; ii++)
		{
			sim.attachOrganism(gen[ii]);
			// let organism flail around a while
			for (ik=0; ik<5000&&!sim.converged; ik++)
				sim.stepSimulation(0.01, 1);

			gen[ii]->fitness = sim.getFitness();
			cout << "FIT " << ij<< " "<<ii<<" " << gen[ii]->fitness << " " << gen[ii]->generation << " " << ik << endl;
			sim.reset();
		}

		sort(gen.begin(), gen.end(), compareOrganisms);

		cout << endl;
		cout << "MAX " << ij << " " << gen[0]->fitness << " " << endl << endl;


		// create a new generation
		for (ii=0; ii<GEN_SIZE; ii++)
		{
			if (((float)ii)/GEN_SIZE < GEN_BABIES)
			{
				// pick two fit parents
				parentA = -1;
				parentB = -1;
				while (parentA < 0 || parentA >= GEN_SIZE)
					parentA = (int)dist_exp(eng);
				while (parentB==parentA || parentB < 0 || parentB >= GEN_SIZE)
					parentB = (int)dist_exp(eng);
				cout << "mating " << parentA << " with " << parentB << endl;
				nextgen[ii] = gen[parentA]->makeBabyWith(gen[parentB], 0.001);
			} else if (((float)ii)/GEN_SIZE < GEN_CLONES+GEN_BABIES) {
				// pick a fit parent
				parentA = -1;
				while (parentA < 0 || parentA >= GEN_SIZE)
					parentA = (int)dist_exp(eng);
				cout << "cloning " << parentA << endl;
				nextgen[ii] = gen[parentA]->clone(0.05);
			} else {
				// completely random organism
				nextgen[ii] = new organism(true);
			}
		}
		// copy next gen to curr gen
		for (ii=0; ii<GEN_SIZE; ii++)
		{
			delete gen[ii];
			gen[ii] = nextgen[ii];
		}
	}

	bob = gen[0]->clone(0.0);
	sim.attachOrganism(bob);
	fname = "brain01";
	bob->saveToFile(fname.c_str());
	// clear out generation
	for (ii=0; ii<GEN_SIZE; ii++)
		delete gen[ii];
  //return 0;
	return glutmain(argc, argv, 1280, 960, "HexSim", &sim);
}
