using namespace std;
#include <algorithm>
#include "organism.h"
#include "HexSim.h"
#include "GlutStuff.h"

bool compareOrganisms (organism *a, organism *b) {return (a->fitness>b->fitness);}

int main(int argc,char* argv[])
{
	int ii, ij, ik;
  HexSim sim;
	vector<organism*> gen, nextgen;
	organism *bob;

  sim.initPhysics();


	// populate first generation
	for (ii=0; ii<GEN_SIZE; ii++)
	{
		gen.push_back(new organism(true));
	}
	nextgen.resize(GEN_SIZE);

	// iterate generations
	for (ij=0; ij<10; ij++)
	{
		// march through each organism in the current generation
		for (ii=0; ii<GEN_SIZE; ii++)
		{
			sim.attachOrganism(gen[ii]);
			// let organism flail around a while
			for (ik=0; ik<5000; ik++)
				if (!sim.converged)
					sim.stepSimulation(0.01, 1);

			gen[ii]->fitness = sim.getFitness();
			cout << "FIT " << gen[ii]->fitness << endl;
			sim.reset();
		}

		sort(gen.begin(), gen.end(), compareOrganisms);

		cout << endl;
		cout << "MAX " << ij << " " << gen[0]->fitness << " " << endl << endl;

		// create a new generation
		ik = 0;
		for (ii=0; ii<GEN_SIZE; ii++)
		{
			if (((float)ii)/GEN_SIZE < GEN_CLONES)
			{
				nextgen[ii] = gen[ik]->clone(0.01);
				ik ++;
				if (ik > GEN_SIZE/2) ik = 0;
			}
			else
				nextgen[ii] = new organism(true);
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
	// clear out generation
	for (ii=0; ii<GEN_SIZE; ii++)
		delete gen[ii];
  //return 0;
	return glutmain(argc, argv, 1280, 960, "HexSim", &sim);
}
