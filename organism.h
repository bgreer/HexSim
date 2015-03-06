#ifndef ORGANISM_H
#define ORGANISM_H

using namespace std;
#include <iostream>
#include <random>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include "tuning.h"

#define NUM_OUTPUTS NUM_LEGS*3*FOURIER_PARAMS

class organism
{
public:
	double fitness;
	long iter;

	// controlling parameters
	double *params;
	// data out:
	double *outputs;
	
	bool alloc;

	// actual number of inputs will be NUM_INPUTS * numhistory
	// the 0th history is the current inputs
	// the coefs between layer i and layer j will have (n_i+1)x(n_j) numbers
	// the extra is the bias node (=1)

	// CONSTRUCTOR
	organism (double mutate)
	{
		iter = 0;
		alloc = false;
		init();
		randomizeCoefs(mutate);
	}

	// DESTRUCTOR
	~organism ()
	{
		int ii, ij, ik;
		if (alloc)
		{
			delete [] params;
			delete [] outputs;
			alloc = false;
		}
	}
/*
	void saveToFile (const char *fname)
	{
		size_t size;
		char *buffer;
		ofstream file;

		buffer = packData(&size);

		cout << "Saving size " << size << " to " << fname << endl;

		file.open(fname, ios::out | ios::binary);
		file.write(buffer,size*sizeof(char));
		file.close();

		delete [] buffer;
	}

	
	void readFromFile (char *fname)
	{
		int ii, ij, numparams;
		size_t sf, si;
		char *sizebuffer, databuffer;
		ifstream file;
		
		sf = sizeof(float);
		si = sizeof(int);
		sizebuffer = new char [1024];
		file.open(fname, ios::in | ios::binary);
		// numparams
		file.read(sizebuffer, si);
		memcpy(&numparams, sizebuffer, si);
		// errors...
		outputs = new float [NUM_OUTPUTS];
		// numhistory
		history = new float [NUM_INPUTS*(NUM_HISTORY+1)];
		// numnodes[]
		file.read(sizebuffer, si*numlayers);
		memcpy(numnodes, sizebuffer, si*numlayers);
		coefs_input = new float* [numnodes[0]];
		for (ii=0; ii<numnodes[0]; ii++)
			coefs_input[ii] = new float [NUM_INPUTS*NUM_HISTORY+1];
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

		// read coefs
		for (ii=0; ii<numnodes[0]; ii++)
			file.read(reinterpret_cast<char*>(coefs_input[ii]),sf*(NUM_INPUTS*NUM_HISTORY+1));
		for (ii=0; ii<NUM_OUTPUTS; ii++)
			file.read(reinterpret_cast<char*>(coefs_output[ii]),sf*(numnodes[numlayers-1]+1));
		for (ii=0; ii<numlayers-1; ii++)
			for (ij=0; ij<numnodes[ii+1]; ij++)
				file.read(reinterpret_cast<char*>(coefs[ii][ij]),sf*(numnodes[ii]+1));


		// all done
		file.close();

		alloc = true;
		delete [] sizebuffer;
	}

	char *packData (size_t *s)
	{
		char *buffer;
		size_t si, sf, size;
		int ii, ij, offset;

		// make room for info
		si = sizeof(int);
		sf = sizeof(float);
		size = si + si*numlayers
			+ sf*(NUM_INPUTS*NUM_HISTORY+1)*numnodes[0]
			+ sf*((numnodes[numlayers-1]+1)*NUM_OUTPUTS);
		for (ii=0; ii<numlayers-1; ii++)
			size += sf*(numnodes[ii+1])*(numnodes[ii]+1);
		buffer = new char [size];
		*s = size;

		// load info into buffer
		offset = 0;
		memcpy(buffer+offset,&numlayers,si);
		offset += si;
		memcpy(buffer+offset,numnodes,si*numlayers);
		offset += si*numlayers;
		// copy input coefs
		for (ii=0; ii<numnodes[0]; ii++)
		{
			memcpy(buffer+offset,coefs_input[ii],sf*(NUM_INPUTS*NUM_HISTORY+1));
			offset += sf*(NUM_INPUTS*NUM_HISTORY+1);
		}
		// copy output coefs
		for (ii=0; ii<NUM_OUTPUTS; ii++)
		{
			memcpy(buffer+offset,coefs_output[ii],sf*(numnodes[numlayers-1]+1));
			offset += sf*(numnodes[numlayers-1]+1);
		}
		// copy middle coefs
		for (ii=0; ii<numlayers-1; ii++)
		{
			for (ij=0; ij<numnodes[ii+1]; ij++)
			{
				memcpy(buffer+offset,coefs[ii][ij],sf*(numnodes[ii]+1));
				offset += sf*(numnodes[ii]+1);
			}
		}

		return buffer;
	}

	size_t getHash ()
	{
		char *buffer;
		string str;
		size_t ret, size;
		hash<string> str_hash;

		buffer = packData(&size);

		// compute hash
		str = buffer;
		ret = str_hash(str);
		delete [] buffer;
		return ret;
	}*/

	// assuming inputs[] and coefs[] have been set, computes proper output
	void computeOutputs (double theta)
	{
		int ii, ij;
		double a, b;

		for (ii=0; ii<NUM_OUTPUTS; ii++)
		{
			outputs[ii] = params[ii*FOURIER_PARAMS];
			for (ij=0; ij<NUM_MODES; ij++)
			{
				a = params[ii*FOURIER_PARAMS+1+ij*2];
				b = params[ii*FOURIER_PARAMS+2+ij*2];
				outputs[ii] += a*cos(theta*(ij+1)) + b*sin(theta*(ij+1));
			}
		}
	}



	void init ()
	{

		params = new double [FOURIER_PARAMS*NUM_OUTPUTS];
		memset(params, 0x00, NUM_OUTPUTS*FOURIER_PARAMS*sizeof(double));
		// in/out
		outputs = new double [NUM_OUTPUTS];
		memset(outputs,0x00,NUM_OUTPUTS*sizeof(double));
	
		alloc = true;
	}

	void randomizeCoefs (double variance)
	{
		int ii, ij, ik;
		std::mt19937 eng((std::random_device())());
		if (alloc)
		{
			// initialize coefs randomly
			std::normal_distribution<double> dist_norm(0.0, variance);
			for (ii=0; ii<FOURIER_PARAMS; ii++)
				params[ii] = dist_norm(eng);
		}
	}


	organism* makeBabyWith (organism *mate, double mutation)
	{
		organism *ret;

		ret = new organism(0.0);

		// implement

		return ret;
	}





	organism* clone (double mutation)
	{
		int ii, ij, ik;
		int common, common2, common3;
		organism *ret;

		ret = new organism(0.0);

		// implement

		return ret;
	}
};

#endif
