#ifndef ORGANISM_H
#define ORGANISM_H

using namespace std;
#include <iostream>
#include <random>
#include <string.h>
#include "tuning.h"

#define NUM_INPUTS NUM_LEGS*2
#define NUM_OUTPUTS NUM_LEGS*2

// TODO:
// save / load brain from file
// clone organism ( w/ mutation rate )
// combine two organisms to form new organism (baby making)

class organism
{
public:
	double fitness;
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
			delete [] numnodes;
			alloc = false;
		}
	}

	size_t getHash ()
	{
		int ii, offset;
		char *buffer;
		string str;
		size_t ret, si, sf, size;
		hash<string> str_hash;

		// make room for info
		si = sizeof(int);
		sf = sizeof(float);
		size = si*2 + si*numlayers
			+ sf*(NUM_INPUTS*numhistory+1)*numnodes[0]
			+ sf*((numnodes[numlayers-1]+1)*NUM_OUTPUTS);
		buffer = new char [size];

		// load info into buffer
		offset = 0;
		memcpy(buffer+offset,&numlayers,si);
		offset += si;
		memcpy(buffer+offset,&numhistory,si);
		offset += si;
		memcpy(buffer+offset,&numnodes,si*numlayers);
		offset += si*numlayers;
		// copy input coefs
		for (ii=0; ii<numnodes[0]; ii++)
		{
			memcpy(buffer+offset,coefs_input[ii],sf*(NUM_INPUTS*numhistory+1));
			offset += sf*(NUM_INPUTS*numhistory+1);
		}
		// copy output coefs
		for (ii=0; ii<NUM_OUTPUTS; ii++)
		{
			memcpy(buffer+offset,coefs_output[ii],sf*(numnodes[numlayers-1]+1));
			offset += sf*(numnodes[numlayers-1]+1);
		}

		// compute hash
		str = buffer;
		ret = str_hash(str);
		delete [] buffer;
		return ret;
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
				node[0][ii] += coefs_input[ii][ij+NUM_INPUTS] * history[ij];
			// bias
			node[0][ii] += coefs_input[ii][NUM_INPUTS*numhistory];
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

		// enforce upper bounds
		if (numlayers > MAX_LAYERS) numlayers = MAX_LAYERS;
		if (numhistory > MAX_HISTORY) numhistory = MAX_HISTORY;
	
		// in/out
		inputs = new float [NUM_INPUTS];
		outputs = new float [NUM_OUTPUTS];
		history = new float [NUM_INPUTS*(numhistory+1)];
		memset(inputs,0x00,NUM_INPUTS*sizeof(float));
		memset(outputs,0x00,NUM_OUTPUTS*sizeof(float));
		memset(history,0x00,NUM_INPUTS*(numhistory+1)*sizeof(float));
	
		// nodes for each layer
		std::poisson_distribution<> dist_poi(NUM_INPUTS*(numhistory+1));
		numnodes = new int[numlayers];
		for (ii=0; ii<numlayers; ii++)
		{
			numnodes[ii] = (int)dist_poi(eng);
			if (numnodes[ii] < MIN_NODES) numnodes[ii] = MIN_NODES;
			if (numnodes[ii] > MAX_NODES) numnodes[ii] = MAX_NODES;
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
		std::normal_distribution<float> dist_norm(0.0, 20.0);
		for (ii=0; ii<numnodes[0]; ii++)
			for (ij=0; ij<NUM_INPUTS*numhistory+1; ij++)
				coefs_input[ii][ij] = dist_norm(eng);
		for (ii=0; ii<NUM_OUTPUTS; ii++)
			for (ij=0; ij<numnodes[numlayers-1]+1; ij++)
				coefs_output[ii][ij] = dist_norm(eng);
		for (ii=0; ii<numlayers-1; ii++)
			for (ij=0; ij<numnodes[ii+1]; ij++)
				for (ik=0; ik<numnodes[ii]+1; ik++)
					coefs[ii][ij][ik] = dist_norm(eng);

		}
	}


	organism* clone (float mutation)
	{
		int ii, ij, ik;
		int common, common2, common3;
		organism *ret;

		ret = new organism(false);

		// set up random numbers
		std::mt19937 eng((std::random_device())());
		std::normal_distribution<float> dist_layers(numlayers, mutation*numlayers);
		std::normal_distribution<float> dist_history(numhistory, mutation*numhistory);

		ret->numlayers = (int)round(dist_layers(eng));
		ret->numhistory = (int)round(dist_history(eng));
		// enforce bounds
		if (ret->numlayers < MIN_LAYERS) ret->numlayers = MIN_LAYERS;
		if (ret->numhistory < MIN_HISTORY) ret->numhistory = MIN_HISTORY;
		if (ret->numlayers > MAX_LAYERS) ret->numlayers = MAX_LAYERS;
		if (ret->numhistory > MAX_HISTORY) ret->numhistory = MAX_HISTORY;

		if (numlayers != ret->numlayers)
			cout << "change in layers: " << numlayers << " -> " << ret->numlayers << endl;
		if (numhistory != ret->numhistory)
			cout << "change in history: " << numhistory << " -> " << ret->numhistory << endl;
	
		// in/out
		ret->inputs = new float [NUM_INPUTS];
		ret->outputs = new float [NUM_OUTPUTS];
		ret->history = new float [NUM_INPUTS*(ret->numhistory+1)];
		memset(ret->inputs,0x00,NUM_INPUTS*sizeof(float));
		memset(ret->outputs,0x00,NUM_OUTPUTS*sizeof(float));
		memset(ret->history,0x00,NUM_INPUTS*(ret->numhistory+1)*sizeof(float));

		// nodes for each layer
		ret->numnodes = new int[ret->numlayers];
		common = min(ret->numlayers, numlayers);
		for (ii=0; ii<common; ii++)
		{
			std::normal_distribution<float> dist_nodes(numnodes[ii], mutation*numnodes[ii]);
			ret->numnodes[ii] = (int)round(dist_nodes(eng));
			if (ret->numnodes[ii] < MIN_NODES) ret->numnodes[ii] = MIN_NODES;
			if (ret->numnodes[ii] > MAX_NODES) ret->numnodes[ii] = MAX_NODES;
		}
		for (ii=common; ii<ret->numlayers; ii++)
		{
			std::normal_distribution<float> dist_nodes(numnodes[common-1], mutation*numnodes[common-1]);
			ret->numnodes[ii] = (int)round(dist_nodes(eng));
			if (ret->numnodes[ii] < MIN_NODES) ret->numnodes[ii] = MIN_NODES;
			if (ret->numnodes[ii] > MAX_NODES) ret->numnodes[ii] = MAX_NODES;
		}

		// coef memory is aligned [output][input]
		// make room for coefs
		ret->coefs_input = new float* [ret->numnodes[0]];
		for (ii=0; ii<ret->numnodes[0]; ii++)
			ret->coefs_input[ii] = new float [NUM_INPUTS*ret->numhistory+1];
		ret->coefs_output = new float* [NUM_OUTPUTS];
		for (ii=0; ii<NUM_OUTPUTS; ii++)
			ret->coefs_output[ii] = new float [ret->numnodes[ret->numlayers-1]+1];
		ret->coefs = new float** [ret->numlayers-1];
		for (ii=0; ii<ret->numlayers-1; ii++)
		{
			ret->coefs[ii] = new float* [ret->numnodes[ii+1]];
			for (ij=0; ij<ret->numnodes[ii+1]; ij++)
				ret->coefs[ii][ij] = new float [ret->numnodes[ii]+1];
		}

		ret->node = new float* [ret->numlayers];
		for (ii=0; ii<ret->numlayers; ii++)
			ret->node[ii] = new float [ret->numnodes[ii]];

		ret->iter = 0;
		ret->alloc = true;

		std::normal_distribution<float> dist_norm(0.0, 20.0);
		std::lognormal_distribution<float> dist_mutate(1.0, mutation);

		common = min(ret->numnodes[0], numnodes[0]);
		for (ii=0; ii<common; ii++)
		{
			common2 = NUM_INPUTS*min(ret->numhistory, numhistory)+1;
			for (ij=0; ij<common2; ij++)
				ret->coefs_input[ii][ij] = coefs_input[ii][ij] * dist_mutate(eng);
			for (ij=common2; ij<NUM_INPUTS*ret->numhistory+1; ij++)
				ret->coefs_input[ii][ij] = dist_norm(eng);
		}
		for (ii=common; ii<ret->numnodes[0]; ii++)
			for (ij=0; ij<NUM_INPUTS*ret->numhistory+1; ij++)
				ret->coefs_input[ii][ij] = dist_norm(eng);
		for (ii=0; ii<NUM_OUTPUTS; ii++)
		{
			common = min(ret->numnodes[ret->numlayers-1], numnodes[numlayers-1])+1;
			for (ij=0; ij<common; ij++)
				ret->coefs_output[ii][ij] = coefs_output[ii][ij]*dist_mutate(eng);
			for (ij=common; ij<ret->numnodes[ret->numlayers-1]+1; ij++)
				ret->coefs_output[ii][ij] = dist_norm(eng);
		}
		common = min(ret->numlayers, numlayers)-1;
		for (ii=0; ii<common; ii++)
		{
			common2 = min(ret->numnodes[ii+1], numnodes[ii+1]);
			for (ij=0; ij<common2; ij++)
			{
				common3 = min(ret->numnodes[ii], numnodes[ii])+1;
				for (ik=0; ik<common3; ik++)
					ret->coefs[ii][ij][ik] = coefs[ii][ij][ik]*dist_mutate(eng);
				for (ik=common3; ik<ret->numnodes[ii]+1; ik++)
					ret->coefs[ii][ij][ik] = dist_norm(eng);
			}
			for (ij=common2; ij<ret->numnodes[ii+1]; ij++)
				for (ik=0; ik<ret->numnodes[ii]+1; ik++)
					ret->coefs[ii][ij][ik] = dist_norm(eng);
		}
		for (ii=common; ii<ret->numlayers-1; ii++)
			for (ij=0; ij<ret->numnodes[ii+1]; ij++)
				for (ik=0; ik<ret->numnodes[ii]+1; ik++)
					ret->coefs[ii][ij][ik] = dist_norm(eng);

		return ret;
	}
};

#endif
