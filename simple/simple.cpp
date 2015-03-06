using namespace std;
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <fstream>
#include <random>

#include "organism.h"



bool compareOrganisms (organism *a, organism *b) {return (a->getFitness()>b->getFitness());}

int main (int argc, char *argv[])
{
	int ii, ij, ik, parentA, parentB, genind;
	double x, genpos;
	ofstream file;
	vector<organism*> gen[NUM_COMMUNITIES];
	vector<organism*> newgen[NUM_COMMUNITIES];
	organism *best;
	std::mt19937 eng((std::random_device())());
	std::normal_distribution<double> dist_norm(0.0,1.0);
	std::uniform_int_distribution<int> dist_int(0,GEN_SIZE/2);
	std::uniform_real_distribution<double> dist_real(0,1);

	// create first generation
	for (ii=0; ii<NUM_COMMUNITIES; ii++)
	{
		for (ij=0; ij<GEN_SIZE; ij++)
			gen[ii].push_back(new organism(MUTATION_NEW, &eng));
		newgen[ii].resize(GEN_SIZE);
	}
	
	best = gen[0][0]->clone(0.0);

	file.open("fitness");

	// evolve
	for (ii=0; ii<NUM_GENS; ii++)
	{
		cout << "gen " << ii << endl;
		// sort by fitness
		for (ij=0; ij<NUM_COMMUNITIES; ij++)
		{
			sort(gen[ij].begin(), gen[ij].end(), compareOrganisms);
			delete best;
			best = gen[ij][0]->clone(0.0);

			// print fitness for visualization
			for (ik=0; ik<GEN_SIZE; ik++)
				file << ii << " " << ij << " " << gen[ij][ik]->getFitness() << endl;
		}

		// create new generation
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
			// delete old generation, copy new one over
			for (ik=0; ik<GEN_SIZE; ik++)
			{
				delete gen[ij][ik];
				gen[ij][ik] = newgen[ij][ik];
			}
		}
	}

	file.close();


	file.open("plot");
	// print best one
	for (ii=0; ii<128; ii++)
	{
		x = ii/128.;
		file << x << " " << best->getValue(x) << endl;
	}
	file.close();

	delete best;

	return 0;
}
