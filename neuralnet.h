// Eric Butler
// Ghosts! game player
// Based on the orignal code for CS340.
// Version: 2.2.0
// Started on: Tue Dec 20 13:42:45 EST 2011
// Copyright 2011

#include <iostream>
#ifndef neuralnet_h
#define neuralnet_h

#include <vector>
using namespace std;

struct neuron {
	vector<double> inputWeights;
	int numInputs;
	neuron( int );
};
struct neuronlayer {
	int numNeurons;
	vector<neuron> neurons;
	neuronlayer( int, int );
};

class neuralnet {
	private:
		vector<neuronlayer> neuronLayers;
		int ins;
		int outs;
		int hiddenLayers;
		int * neuronsPerLayer;
		double sigmoid( double, double );

	public:
		neuralnet( int, int, int, int[] );
		vector<double> getWeights() const;
		int getNumWeights() const;
		void pushWeights( vector<double> &, int );
		vector<double> infer( vector<double> & );

}; // end neuralnet class

#endif

