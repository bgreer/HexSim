using namespace std;
#include <algorithm>
#include <sstream>
#include "organism.h"
#include "HexSim.h"
#include "GlutStuff.h"

bool compareOrganisms (organism *a, organism *b) {return (a->fitness>b->fitness);}

int main(int argc,char* argv[])
{
	int ii, ij, ik, parentA, parentB;
	stringstream fname;
  HexSim sim;
	vector<organism*> gen, nextgen;
	organism *bob;
	vector<float> vals, probs;

  sim.initPhysics();

	// initialize random number generators
	std::mt19937 eng((std::random_device())());
	std::exponential_distribution<float> dist_exp(3.0/GEN_SIZE);
	vals.resize(GEN_SIZE+1);
	for (ii=0; ii<GEN_SIZE+1; ii++)
		vals[ii] = (float)ii;
	probs.resize(GEN_SIZE);

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

		// save best
		fname.str(std::string());
		fname << "brain_" << ij;
		gen[0]->saveToFile(fname.str().c_str());

		// create a new generation
		for (ii=0; ii<GEN_SIZE; ii++)
			probs[ii] = pow(max(gen[ii]->fitness, 0.01),SELECTION_STRENGTH);

		
		std::piecewise_constant_distribution<float> 
			dist_fit(vals.begin(),vals.end(),probs.begin());
		for (ii=0; ii<GEN_SIZE; ii++)
		{
			if (((float)ii)/GEN_SIZE < GEN_BABIES)
			{
				// pick two fit parents
				parentA = -1;
				parentB = -1;
				while (parentA < 0 || parentA >= GEN_SIZE)
					parentA = (int)dist_fit(eng);
				while (parentB==parentA || parentB < 0 || parentB >= GEN_SIZE)
					parentB = (int)dist_fit(eng);
				cout << "mating " << parentA << " with " << parentB << endl;
				nextgen[ii] = gen[parentA]->makeBabyWith(gen[parentB], 0.001);
			} else if (((float)ii)/GEN_SIZE < GEN_CLONES+GEN_BABIES) {
				// pick a fit parent
				parentA = -1;
				while (parentA < 0 || parentA >= GEN_SIZE)
					parentA = (int)dist_fit(eng);
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
	// clear out generation
	for (ii=0; ii<GEN_SIZE; ii++)
		delete gen[ii];
  //return 0;
	return glutmain(argc, argv, 1280, 960, "HexSim", &sim);
}
