// Eric Butler
// Ghosts! game neural network
// Based on the orignal code for CS340.
// Version: 2.2.0
// Started on: Tue Dec 20 13:42:45 EST 2011
// Copyright 2011

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include "neuralnet.h"

#define RANDDOUBLE ((double)rand()/(double)RAND_MAX)

// neuron cstor
// @ ins - number of inputs for this neuron
neuron::neuron( int ins )
	: numInputs( ins + 1 ) { // add one for the bias
	for ( int i = 0; i < this->numInputs; i++ )
		this->inputWeights.push_back( 1.0 );
} // end neuron struct cstor

// neuronlayer cstor
// @ num - number of neurons in this layer
// @ insPerNeuron - inputs per each neuron 
neuronlayer::neuronlayer( int num, int insPerNeuron )
	: numNeurons( num ) {
	for ( int i = 0; i < numNeurons; i++ )
		this->neurons.push_back( neuron( insPerNeuron ) );
} // end neuron struct cstor

// neuralnet cstor
// @ in - number of inputs
// @ out - number of output neurons
// @ hl - number of hidden layers
// @ npl - array of, each index specifiying number of neurons at that layer 
neuralnet::neuralnet( int in, int out, int hl, int npl[] ) 
	: ins ( in ), outs( out ), hiddenLayers( hl ), neuronsPerLayer( npl ) {
		srand( time( NULL ) ); // relatively random seed
		// create the layers of the network
		this->neuronLayers.push_back( neuronlayer( this->neuronsPerLayer[ 0 ], this->ins ) ); // create first hidden layer
		for ( int i = 0; i < this->hiddenLayers - 1; ++i ) { // additional hidden layers
			this->neuronLayers.push_back( neuronlayer( this->neuronsPerLayer[ i + 1 ], this->neuronsPerLayer[ i ] ) );
		}
		// create output layer
		this->neuronLayers.push_back( neuronlayer( this->outs, this->neuronsPerLayer[ this->hiddenLayers - 1 ] ) );
} // end cstor

// infer - uses the neural network to produce some results
// @ inputs - vector of inputs
// PRE: inputs should be the same size as this->ins
vector<double> neuralnet::infer( vector<double> & inputs ) {
	vector<double> outputs;
	int curWeight;
	if ( inputs.size() != this->ins ) { // first check that we have the correct number of inputs
		return outputs;
	}
	for ( int i = 0; i < this->hiddenLayers + 1; i++ ) { // for each layer
		if ( i > 0 )
			inputs = outputs;
		outputs.clear();
		curWeight = 0;
		// for each neuron sum the (inputs * corresponding weights). Throw 
		// the total at our sigmoid function to get the output.
		for ( int j = 0; j < this->neuronLayers[ i ].numNeurons; j++) {
			double netinput = 0;
			for ( int k = 0 ; k < this->neuronLayers[ i ].neurons[ j ].numInputs - 1; k++ ) { // for each weight
				// sum the weights x inputs
				netinput += this->neuronLayers[ i ].neurons[ j ].inputWeights[ k ] * inputs[ curWeight ];
				curWeight++;
			} // end for each weight
			// subtract off the bias for activation threshold. damn you math, signed angry chuck
			// sigmoid means S curve, damn you maths!
			netinput += this->neuronLayers[ i ].neurons[ j ].inputWeights[ this->neuronLayers[ i ].neurons[ j ].numInputs - 1 ] * (-1);
			outputs.push_back( this->sigmoid( netinput, 1 ) ); // activation response, change 1 to play with
			curWeight = 0; // reset curWeight
		} // end for each neuron
	} // end for each layer
	return outputs;
} // end infer

// getWeights - returns the weights being used by this neural network
vector<double> neuralnet::getWeights() const {
	vector<double> weights;
	for ( int i = 0; i < this->hiddenLayers + 1; i++ ) { // for each layer
		for ( int j = 0; j < this->neuronLayers[ i ].numNeurons; j++) { // for each neuron
			for ( int k = 0 ; k < this->neuronLayers[ i ].neurons[ j ].numInputs; k++ ) // for each weight
				weights.push_back( this->neuronLayers[ i ].neurons[ j ].inputWeights[ k ]);
		} // end for each neuron
	} // end for each layer
	return weights;
} // end getWeights

// pushWeights - puts weights on the constructed neuralNetwork
// @ weights - weights to put on the network
// @ ignoreBase - ignore the n first elements (used for something else)
void neuralnet::pushWeights( vector<double> & weights, int ignoreBase ) {
	int curWeight = ignoreBase; // start at ignore base, we don't need the first ignore base weights
	for ( int i = 0; i < this->hiddenLayers + 1; i++ ) { // for each layer
		for ( int j = 0; j < this->neuronLayers[ i ].numNeurons; j++) { // for each neuron
			for ( int k = 0 ; k < this->neuronLayers[ i ].neurons[ j ].numInputs; k++ ) { // for each weight
				if ( curWeight > weights.size() - 1 ) { // how does this happen?
					cout << "playing randomly" << endl;
					this->neuronLayers[ i ].neurons[ j ].inputWeights[ k ] = RANDDOUBLE - RANDDOUBLE;
				} else {
					this->neuronLayers[ i ].neurons[ j ].inputWeights[ k ] = weights[ curWeight ];
				}
				curWeight++;
			} // end for each weight
		} // end for each neuron
	} // end for each layer
} // end pushWeights

// getNumWeights - returns the number of weights this neural network requires
int neuralnet::getNumWeights() const {
	int weights = 0;
	for ( int i = 0; i < this->hiddenLayers + 1; i++ ) { // for each layer
		for ( int j = 0; j < this->neuronLayers[ i ].numNeurons; j++) { // for each neuron
			for ( int k = 0 ; k < this->neuronLayers[ i ].neurons[ j ].numInputs; k++ ) // for each weight
				weights++;
		} // end for each neuron
	} // end for each layer
	return weights;
} // end getNumWeights

// sigmoid - helper function, returns a number from -1.0 to 1.0 for neuron firings
// @ netinput - input coming into neuron
// @ response - activation response
double neuralnet::sigmoid( double netinput, double response ) {
	return ( 1 / ( 1 + exp( -netinput / response ) ) );
} // end sigmoid
