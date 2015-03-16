using namespace std;
#include <algorithm>
#include <iostream>
#include <sstream>
#include "organism.h"
#include "HexSim.h"
#include "GlutStuff.h"

bool compareOrganisms (organism *a, organism *b) {return (a->fitness>b->fitness);}

int main(int argc,char* argv[])
{
	int ii, ij, ik, il;
	int genind, parentA, parentB;
	double genpos;
	stringstream fname;
  HexSim sim;
	vector<organism*> gen[NUM_COMMUNITIES];
	vector<organism*> newgen[NUM_COMMUNITIES];
	organism *best;
	vector<float> vals, probs;
	ofstream file;

  sim.initPhysics();

	// initialize random number generators
	std::mt19937 eng((std::random_device())());
	std::normal_distribution<double> dist_norm(0.0,1.0);
	std::uniform_int_distribution<int> dist_int(0,GEN_SIZE/2);
	std::uniform_real_distribution<double> dist_real(0,1);


	// populate first generation
	for (ii=0; ii<NUM_COMMUNITIES; ii++)
	{
		for (ij=0; ij<GEN_SIZE; ij++)
			gen[ii].push_back(new organism(MUTATION_NEW, &eng));
		newgen[ii].resize(GEN_SIZE);
	}

	file.open("fitness");

	// iterate generations
	for (ii=0; ii<NUM_GENS; ii++)
	{
		cout << "gen " << ii << endl;
		for (ij=0; ij<NUM_COMMUNITIES; ij++)
		{
			for (ik=0; ik<GEN_SIZE; ik++)
			{
				// evolve to evaluate fitness
				sim.attachOrganism(gen[ij][ik]);
				for (il=0; il<1000&&!sim.converged; il++)
					sim.stepSimulation(0.01, 1);
				gen[ij][ik]->fitness = sim.getFitness();
				file << ii << " " << ik << " " << gen[ij][ik]->fitness << endl;
				sim.reset();
			}

			// sort generation by fitness
			sort(gen[ij].begin(), gen[ij].end(), compareOrganisms);
			if (ij==0)
			{
				fname.str(std::string());
				fname << "brain_" << ii;
				gen[ij][0]->saveToFile(fname.str().c_str());
			}

			// create a new generation
			for (ij=0; ij<NUM_COMMUNITIES; ij++)
			{
				for (ik=0; ik<GEN_SIZE; ik++)
				{
					// pick parents
					parentA = dist_int(eng);
					parentB = parentA;
					while (parentB == parentA)
						parentB = dist_int(eng);
					// chance of cross-community breeding
					genind = ij;
					if (dist_real(eng) < COMMUNITY_MIXING)
						genind = (int)(dist_real(eng)*NUM_COMMUNITIES);

					// decide what to do with them
					genpos = ((double)ik)/((double)GEN_SIZE);
					if (genpos < FRAC_BABIES)
					{
						newgen[ij][ik] = gen[ij][parentA]->makeBabyWith(gen[genind][parentB], 
								MUTATION_BABY);
					} else if (genpos < FRAC_BABIES + FRAC_CLONES) {
						newgen[ij][ik] = gen[ij][parentA]->clone(MUTATION_CLONE);
					} else {
						newgen[ij][ik] = new organism(MUTATION_NEW, &eng);
					}
				}
				// delete old generation, copy over new one
				for (ik=0; ik<GEN_SIZE; ik++)
				{
					delete gen[ij][ik];
					gen[ij][ik] = newgen[ij][ik];
				}

			}

		}
	}

	file.close();

	best = gen[0][0]->clone(0.0);
	sim.attachOrganism(best);
	// clear out generation
  //return 0;
	return glutmain(argc, argv, 1280, 960, "HexSim", &sim);
}
