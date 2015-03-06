using namespace std;

#include <random>

#define GEN_SIZE 100
#define NUM_GENS 100
#define NUM_COMMUNITIES 1
#define COMMUNITY_MIXING 1.0

// percentages
#define FRAC_BABIES 0.90
#define FRAC_CLONES 0.10
#define FRAC_RANDOM 0.00

// mutation rates
#define MUTATION_NEW 1.0
#define MUTATION_BABY 0.01
#define MUTATION_CLONE 0.01

#define PI 3.14159265359
#define NUM_MODES 16
#define NUM_PARAMS (NUM_MODES*2 + 1)
#define INIT_VARIANCE 1.0


class organism
{
private:
	std::mt19937 *eng;
public:	
	double params[NUM_PARAMS];
	organism (double variance, std::mt19937 *eng0) 
	{
		int ii;
		std::normal_distribution<double> dist(0.0,variance);
		eng = eng0;
		for (ii=0; ii<NUM_PARAMS; ii++)
			params[ii] = dist(*eng);
	}

	double getValue (double x)
	{
		int ii;
		double ret;

		ret = params[0];
		for (ii=0; ii<NUM_MODES; ii++)
			ret += (params[1+ii*2] * sin(x*(ii+1)*2.*PI)) + 
					(params[2+ii*2] * cos(x*(ii+1)*2.*PI));
		return ret;
	}

	double getFitness ()
	{
		int ii;
		double ret, x, ref;

		ret = 0.0;
		for (ii=0; ii<512; ii++)
		{
			x = ii/512.;
			ref = 0.0;
			if (x > 0.5) ref = 1.0;
			ret += pow(ref - getValue(x),2.0);
		}
		return 512./ret;
	}

	organism *clone (double mutation)
	{
		int ii;
		std::normal_distribution<double> dist(0.0,mutation);
		organism *ret;
		
		ret = new organism(0.0, eng);
		for (ii=0; ii<NUM_PARAMS; ii++)
			ret->params[ii] = params[ii] + dist(*eng);

		return ret;
	}

	organism *makeBabyWith (organism *mate, double mutation)
	{
		int ii, crossover;
		std::normal_distribution<double> dist(0.0,mutation);
		std::uniform_int_distribution<int> dist_int(1,NUM_PARAMS-2);
		organism *ret;
		ret = new organism(0.0, eng);

		// pick crosspver point
		crossover = dist_int(*eng);

		for (ii=0; ii<crossover; ii++)
			ret->params[ii] = params[ii] + dist(*eng);
		for (ii=crossover; ii<NUM_PARAMS; ii++)
			ret->params[ii] = mate->params[ii] + dist(*eng);

		return ret;
	}
};

