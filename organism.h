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
	std::mt19937 *eng;

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
	organism (double mutate, std::mt19937 *eng0)
	{
		eng = eng0;
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
		int ii, ij, temp;
		size_t sf, si, sd;
		char *sizebuffer, databuffer;
		ifstream file;
		
		sf = sizeof(float);
		sd = sizeof(double);
		si = sizeof(int);
		sizebuffer = new char [1024];
		file.open(fname, ios::in | ios::binary);
		// numparams
		file.read(sizebuffer, si);
		memcpy(&temp, sizebuffer, si);
		if (temp != NUM_MODES)
			cout << "ERROR: incorrect number of modes: " << temp << ", " << NUM_MODES << endl;
		
		// read params
		file.read(reinterpret_cast<char*>(params),sd*NUM_OUTPUTS*FOURIER_PARAMS);
		cout << "size " << NUM_OUTPUTS*FOURIER_PARAMS << endl;
		for (ii=0; ii<NUM_OUTPUTS*FOURIER_PARAMS; ii++)
			cout << ii << " " << params[ii] << endl;

		// all done
		file.close();

		delete [] sizebuffer;
	}

	char *packData (size_t *s)
	{
		char *buffer;
		size_t si, sf, size, sd;
		int ii, ij, offset, temp;

		// num_modes (int)
		// params (floats, (num_modes*2+1)*num_outputs)

		// make room for info
		si = sizeof(int);
		sf = sizeof(float);
		sd = sizeof(double);
		size = si + sd*NUM_OUTPUTS*FOURIER_PARAMS;
		buffer = new char [size];
		*s = size;

		// load info into buffer
		offset = 0;
		temp = NUM_MODES;
		memcpy(buffer+offset,&temp,si);
		offset += si;
		// copy params
		memcpy(buffer+offset,params,sd*NUM_OUTPUTS*FOURIER_PARAMS);

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
	}

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
				a = params[ii*FOURIER_PARAMS+ij*2+1];
				b = params[ii*FOURIER_PARAMS+ij*2+2];
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
		if (alloc)
		{
			// initialize coefs randomly
			std::normal_distribution<double> dist_norm(0.0, variance);
			for (ii=0; ii<FOURIER_PARAMS*NUM_OUTPUTS; ii++)
				params[ii] = dist_norm(*eng);
		}
	}


	organism* makeBabyWith (organism *mate, double mutation)
	{
		int ii, crossover;
		std::normal_distribution<double> dist(0.0,mutation);
		std::uniform_int_distribution<int> dist_int(1,NUM_OUTPUTS*FOURIER_PARAMS-2);
		organism *ret;

		ret = new organism(0.0, eng);

		// pick crosspver point
		crossover = dist_int(*eng);

		for (ii=0; ii<crossover; ii++)
			ret->params[ii] = params[ii] + dist(*eng);
		for (ii=crossover; ii<NUM_OUTPUTS*FOURIER_PARAMS; ii++)
			ret->params[ii] = mate->params[ii] + dist(*eng);


		return ret;
	}





	organism* clone (double mutation)
	{
		int ii, ij, ik;
		int common, common2, common3;
		organism *ret;
		std::normal_distribution<double> dist(0.0,mutation);

		ret = new organism(0.0, eng);

		for (ii=0; ii<NUM_OUTPUTS*FOURIER_PARAMS; ii++)
			ret->params[ii] = params[ii] + dist(*eng);
	

		return ret;
	}
};

#endif
