#ifndef ORGANISM_H
#define ORGANISM_H

using namespace std;
#include <iostream>
#include <random>
#include "tuning.h"

#define NUM_INPUTS NUM_LEGS*2
#define NUM_OUTPUTS NUM_LEGS*2

class organism
{
public:
	long iter;
	// data in/out:
	float *inputs;
	float *outputs;
	
	// neural network size
	int numlayers;
	int *numnodes; // for each layer
	int numhistory; // extra input nodes
	float **coefs_input;
	float **coefs_output;
	float ***coefs;

	// keep track of own history;
	float *history;

	// for temporary storage of node values:
	float **node;

	bool alloc;

	// actual number of inputs will be NUM_INPUTS * numhistory
	// the 0th history is the current inputs
	// the coefs between layer i and layer j will have (n_i+1)x(n_j) numbers
	// the extra is the bias node (=1)

	// CONSTRUCTOR
	organism (bool init_random=false)
	{
		iter = 0;
		alloc = false;
		if (init_random)
		{
			initRandomSize();
			randomizeCoefs();
		}
	}

	// DESTRUCTOR
	~organism ()
	{
		int ii, ij, ik;
		if (alloc)
		{
			for (ii=0; ii<numnodes[0]; ii++)
				delete [] coefs_input[ii];
			delete [] coefs_input;
			for (ii=0; ii<NUM_OUTPUTS; ii++)
				delete [] coefs_output[ii];
			delete [] coefs_output;
			for (ii=0; ii<numlayers-1; ii++)
			{
				for (ij=0; ij<numnodes[ii+1]; ij++)
					delete [] coefs[ii][ij];
				delete [] coefs[ii];
			}
			for (ii=0; ii<numlayers; ii++)
				delete [] node[ii];
			delete [] node;
			delete [] coefs;
			delete [] inputs;
			delete [] outputs;
			delete [] history;
			alloc = false;
		}
	}

	// assuming inputs[] and coefs[] have been set, computes proper output
	void computeOutputs ()
	{
		int ii, ij, ik;

		// compute first layer with inputs
		for (ii=0; ii<numnodes[0]; ii++)
		{
			node[0][ii] = 0.0;
			// real inputs
			for (ij=0; ij<NUM_INPUTS; ij++)
				node[0][ii] += coefs_input[ii][ij] * inputs[ij];
			// history
			for (ij=0; ij<NUM_INPUTS*(numhistory-1); ij++)
				node[0][ii] += coefs_input[ii][ij] * history[ij];
			// bias
			node[0][ii] += coefs_input[ii][numnodes[0]-1];
			node[0][ii] = sigmoid(node[0][ii]);
		}
		for (ii=1; ii<numlayers; ii++)
		{
			// compute the value for each node
			for (ij=0; ij<numnodes[ii]; ij++)
			{
				node[ii][ij] = 0.0;
				for (ik=0; ik<numnodes[ii-1]; ik++)
					node[ii][ij] += coefs[ii-1][ij][ik] * node[ii-1][ik];
				// bias
				node[ii][ij] += coefs[ii-1][ij][numnodes[ii-1]];
				node[ii][ij] = sigmoid(node[ii][ij]);
			}
		}
		// compute final answer
		for (ii=0; ii<NUM_OUTPUTS; ii++)
		{
			outputs[ii] = 0.0;
			// nodes
			for (ij=0; ij<numnodes[numlayers-1]; ij++)
				outputs[ii] += coefs_output[ii][ij] * node[numlayers-1][ij];
			// bias
			outputs[ii] += coefs_output[ii][numnodes[numlayers-1]];
			outputs[ii] = sigmoid(outputs[ii]);
		}

		// at the end, update personal history
		if (iter % HISTORY_SKIP == 0)
		{
			// first move existing history down the array
			for (ii=0; ii<numhistory-2; ii++)
				for (ij=0; ij<NUM_INPUTS; ij++)
					history[(ii+1)*NUM_INPUTS+ij] = history[ii*NUM_INPUTS+ij];
			// then, store most recent history
			for (ii=0; ii<NUM_INPUTS; ii++)
				history[ii] = inputs[ii];
		}
		iter ++;
	}

	float sigmoid (float x)
	{
		return 1.0 / (1.0 + exp(-x));
	}

	void initRandomSize ()
	{
		int ii, ij, ik;

		std::mt19937 eng((std::random_device())());
		std::exponential_distribution<> dist_exp(1);

		// layers, hostory
		numlayers = (int)dist_exp(eng) + MIN_LAYERS;
		numhistory = (int)(dist_exp(eng)) + MIN_HISTORY;

		cout << "numlayers="<<numlayers<<endl;
		cout << "numhistory="<<numhistory<<endl;

		// in/out
		inputs = new float [NUM_INPUTS];
		outputs = new float [NUM_OUTPUTS];
		history = new float [NUM_INPUTS*(numhistory+1)];

		// enforce upper bounds
		if (numlayers > MAX_LAYERS) numlayers = MAX_LAYERS;
		if (numhistory > MAX_HISTORY) numhistory = MAX_HISTORY;
		
		// nodes for each layer
		std::poisson_distribution<> dist_poi(NUM_INPUTS*(numhistory+1));
		numnodes = new int[numlayers];
		for (ii=0; ii<numlayers; ii++)
		{
			numnodes[ii] = (int)dist_poi(eng);

			if (numnodes[ii] > MAX_NODES) numnodes[ii] = MAX_NODES;
			cout << "layer " << ii << " nodes = " <<numnodes[ii]<<endl;
		}

		// coef memory is aligned [output][input]
		// make room for coefs
		coefs_input = new float* [numnodes[0]];
		for (ii=0; ii<numnodes[0]; ii++)
			coefs_input[ii] = new float [NUM_INPUTS*numhistory+1];
		coefs_output = new float* [NUM_OUTPUTS];
		for (ii=0; ii<NUM_OUTPUTS; ii++)
			coefs_output[ii] = new float [numnodes[numlayers-1]+1];
		coefs = new float** [numlayers-1];
		for (ii=0; ii<numlayers-1; ii++)
		{
			coefs[ii] = new float* [numnodes[ii+1]];
			for (ij=0; ij<numnodes[ii+1]; ij++)
				coefs[ii][ij] = new float [numnodes[ii]+1];
		}

		node = new float* [numlayers];
		for (ii=0; ii<numlayers; ii++)
			node[ii] = new float [numnodes[ii]];

		alloc = true;
	}

	void randomizeCoefs ()
	{
		int ii, ij, ik;
		std::mt19937 eng((std::random_device())());
		if (alloc)
		{
		// initialize coefs randomly
		std::normal_distribution<> dist_norm(0.0, 20.0);
		for (ii=0; ii<numnodes[0]; ii++)
			for (ij=0; ij<NUM_INPUTS*numhistory+1; ij++)
				coefs_input[ii][ij] = dist_norm(eng);
		for (ii=0; ii<NUM_OUTPUTS; ii++)
			for (ij=0; ij<numnodes[numlayers-1]; ij++)
				coefs_output[ii][ij] = dist_norm(eng);
		for (ii=0; ii<numlayers-1; ii++)
			for (ij=0; ij<numnodes[ii+1]; ij++)
				for (ik=0; ik<numnodes[ii]; ik++)
					coefs[ii][ij][ik] = dist_norm(eng);

		}
	}
};

#endif
