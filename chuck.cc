// Eric Butler
// Ghosts! game player
// Based on the orignal code for CS340.
// Version: 2.2.0
// Started on: Tue Dec 20 13:42:45 EST 2011
// Copyright 2011

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "chuck.h"
using namespace std;

// neural network parameters:
#define INPUTNODES 36 // number of input nodes
#define OUTPUTNODES 1 // number of output nodes
#define HIDDENLAYERS 2 // number of hidden layers
int NUMPERHIDDEN[ HIDDENLAYERS ] = { 30, 20 }; // number of neurons per layer

// genetic algorithm parameters:
#define POPULATIONSIZE 30 // population size
#define MUTRATE 0.3 // mutation rate, .05 to .3
#define CROSSRATE 0.7 // crossover rate, .7 or maybe higher
#define MAXPERTURB 0.3 // something like .3, maybe a little less
#define GENFILE "players/chuck/geneticData/gen1" // genetics file to write to
#define NUMIGNOREWEIGHTS 24 // number of weights to ignore due to uses for inputs
#define GOODGENESFILE "players/chuck/geneticData/goodgenes" // good genetics file to write to

// alphabeta minimax parameters:
#define PLYDEPTH 3 // look ahead depth (there's inherantly at least one look ahead depth)
#define PRUNING // if defined, this will use pruning

//#define RANDGENES // if defined, it will start from random genes, otherwise, we'll read from enfile
//#define DEBUGMODE // if defined, there is output for debuging 

// default cstor
chuck::chuck()
	: nn( INPUTNODES, OUTPUTNODES, HIDDENLAYERS, NUMPERHIDDEN ),
	g( POPULATIONSIZE, MUTRATE, CROSSRATE, MAXPERTURB, this->nn.getNumWeights() + NUMIGNOREWEIGHTS ) {
		// empty
} // end default cstor 

// cstor
// @ p - which player am I?
chuck::chuck( owner p )
	: whoiam( p ),
	nn( INPUTNODES, OUTPUTNODES, HIDDENLAYERS, NUMPERHIDDEN ),
	g( POPULATIONSIZE, MUTRATE, CROSSRATE, MAXPERTURB, this->nn.getNumWeights() + NUMIGNOREWEIGHTS ) {
		#ifdef RANDGENES 
			this->g.createRandom();
			this->g.writeToFile( GENFILE );
			exit( 0 );
		#else
			this->g.readFromFile( GENFILE );
		#endif
		vector<double> curChromo = this->g.getCurChromo(); // get current chromosome values
		this->nn.pushWeights( curChromo, NUMIGNOREWEIGHTS ); // put our currently being tested chromosome unto neural network
		// there are 16 combinations for enemy values
		// and four for each type of my ghosts, a total of 24 values stored in the chromosome
		// hence the + 24 on the front of the number of weights
		int i;
		for ( i = 0; i < 4; i++ ) {
			for ( int j = 0; j < 4; j++ )
				this->curEneValue[ i ][ j ] = curChromo[ i * 4 + j ];
		}
		for ( i = 0; i < 4; i++ )
			this->curBadValue[ i ] = curChromo[ i + 16 ];
		for ( i = 0; i < 4; i++ )
			this->curGoodValue[ i ] = curChromo[ i + 20 ];
} // end cstor

// dstor
chuck::~chuck() {
	this->g.writeToFile( GENFILE );
} // end dstor 

// setup method - init my board position (statically)
// @ B - board object
// TODO - consider making this genetic
void chuck::setup( ghostboard & B ) {
	if ( this->whoiam == 3 ) { // playerB
		char start[ 4 ][ 2 ] = { 'b', 'y', 'y', 'b', 'b', 'y', 'y', 'b' };
		B.placeghosts( start, whoiam );
	} else  {
		char start[ 4 ][ 2 ] = { 'b', 'y', 'y', 'b', 'b', 'y', 'y', 'b' };
		B.placeghosts( start, whoiam );
	}
} // end setup

// move method - make a move
// @ B - board object to move over
// @ X1 - x position of piece to move
// @ Y1 - y position of piece to move
// @ X2 - new x position of piece
// @ Y2 - new y position of piece 
void chuck::move( ghostboard B, int & X1, int & Y1, int & X2, int & Y2 ) {
	#ifdef DEBUGMODE
		cout << endl;
	#endif
	// x and ys are backwards, lookout!
	vector<chuckChild> node; // current board
	vector<chuckChild> newNode; // children spawn off
	double alpha = -2.0, beta = 2.0; // alpha and beta values, both init as values that will be overridden (-1.0 <= evaluation <= 1.0)
	int currentMoveFrom = 0, currentMoveTo = 0; // saves current move 
	double subTreeValue = -3; // value of subtree
	// encode current board as vector of chuckChild
	if ( this->whoiam == playerA ) { // playerA, keep inputs normal
		for ( Y1 = 0; Y1 < 6; Y1++ ) {
			for ( X1 = 0; X1 < 6; X1++ ) {
				if ( B.getsq( X1, Y1 ).getowner() == 0 ) { // nobody
					node.push_back( noGhost );
				} else if ( B.getsq( X1, Y1 ).getowner() != this->whoiam ) { // not mine
					if ( B.getsq( X1, Y1 ).getcolor() == 1 ) // theirs, good ghost
						node.push_back( themGood );
					else // theirs, bad ghost
						node.push_back( themBad );
				} else if ( B.getsq( X1, Y1 ).getcolor() == 1 ) { // mine, good ghost
					node.push_back( meGood );
				} else { // mine, bad ghost
					node.push_back( meBad );
				}
			}
		} 
	} else { // we're playerB, reverse inputs
		for ( Y1 = 5; Y1 >= 0; Y1-- ) {
			for ( X1 = 5; X1 >= 0; X1-- ) {
				if ( B.getsq( X1, Y1 ).getowner() == 0 ) { // nobody
					node.push_back( noGhost );
				} else if ( B.getsq( X1, Y1 ).getowner() != this->whoiam ) { // not mine
					if ( B.getsq( X1, Y1 ).getcolor() == 1 ) // theirs, good ghost
						node.push_back( themGood );
					else // theirs, bad ghost
						node.push_back( themBad );
				} else if ( B.getsq( X1, Y1 ).getcolor() == 1 ) { // mine, good ghost
					node.push_back( meGood );
				} else { // mine, bad ghost
					node.push_back( meBad );
				}
			}
		} 
	} // end player check
	for ( int i = 0; i < 36 * 4; i++ ) { // for each spot on the board, times four, for each possible direction
		// current piece = i / 4 
		// current direction = i % 4
		if ( node[ i / 4 ] == meGood || node[ i / 4 ] == meBad ) { // its my piece, spawn off children
			if ( i % 4 == 0 && i / 4 > 5 && node[ i / 4 - 6 ] != meGood && node[ i / 4 - 6 ] != meBad ) { // spawn child up
				newNode = node;
				newNode[ i / 4 - 6 ] = newNode[ i / 4 ];
				newNode[ i / 4 ] = noGhost;
				currentMoveFrom = i / 4;
				currentMoveTo = ( i / 4 ) - 6;
				#ifdef DEBUGMODE
					cout << "from " << i/ 4 << " to " << i/4 -6 << endl;
				#endif
				subTreeValue = this->alphaBeta( newNode, PLYDEPTH, alpha, beta, empty );
			} else if ( i % 4 == 1 && i / 4 < 30 && node[ i / 4 + 6 ] != meGood && node[ i / 4 + 6 ] != meBad ) { // spawn child down
				newNode = node;
				newNode[ i / 4 + 6 ] = newNode[ i / 4 ];
				newNode[ i / 4 ] = noGhost;
				currentMoveFrom = i / 4;
				currentMoveTo = ( i / 4 ) + 6;
				#ifdef DEBUGMODE
					cout << "from " << i/ 4<< " to " << i/4 +6<< endl;
				#endif
				subTreeValue = this->alphaBeta( newNode, PLYDEPTH, alpha, beta, empty );
			} else if ( i % 4 == 2 && i / 4 % 6 != 0 && node[ i / 4 - 1 ] != meGood && node[ i / 4 - 1 ] != meBad ) { // spawn child left
				newNode = node;
				newNode[ i / 4 - 1 ] = newNode[ i / 4 ];
				newNode[ i / 4 ] = noGhost;
				currentMoveFrom = i / 4;
				currentMoveTo = ( i / 4 ) - 1;
				#ifdef DEBUGMODE
					cout << "from " << i/ 4<< " to " << i/4 -1<< endl;
				#endif
				subTreeValue = this->alphaBeta( newNode, PLYDEPTH, alpha, beta, empty );
			} else if ( i % 4 == 3 && ( i / 4 + 1 ) % 6 != 0 && node[ i / 4 + 1 ] != meGood && node[ i / 4 + 1 ] != meBad ) { // spawn child right
				newNode = node;
				newNode[ i / 4 + 1 ] = newNode[ i / 4 ];
				newNode[ i / 4 ] = noGhost;
				currentMoveFrom = i / 4;
				currentMoveTo = ( i / 4 ) + 1;
				#ifdef DEBUGMODE
					cout << "from " << i/ 4<< " to " << i/4 +1<< endl;
				#endif
				subTreeValue = this->alphaBeta( newNode, PLYDEPTH, alpha, beta, empty );
			}
			if ( subTreeValue > alpha ) { 
				alpha = subTreeValue;
				// convert current move into actual x y values
				X1 = currentMoveFrom % 6;
				Y1 = currentMoveFrom / 6;
				X2 = currentMoveTo % 6;
				Y2 = currentMoveTo / 6;
				if ( this->whoiam == playerB ) { // playerB, reverse our move
					X1 = 5 - X1;
					Y1 = 5 - Y1;
					X2 = 5 - X2;
					Y2 = 5 - Y2;
				}
				#ifdef DEBUGMODE
					cout << "alpha: " << alpha << " --- X1: " << X1 << " X2: " << X2 << " Y1: " << Y1 << " Y2: " << Y2 << endl;
				#endif
			}
		} else { // its not mine or its empty, skip
			i += 3;
		}
	} // end for each spot
} // end move

// winLose - when I have lost or won, this method signals the genetic object to adjust the fitness accordingly
// @ winner - who the winner is 
void chuck::winLose( owner winner ) {
	if ( winner == this->whoiam ) {
		cout << endl << "I win." << endl;
		this->g.sendFit( 4 );
	} else if ( winner == nowinner ) {
		cout << endl << "Tie." << endl;
		this->g.sendFit( 2 );
	} else {
		cout << endl << "I lose." << endl;
		this->g.sendFit( 1 );
	}
} // end winLose 

// winLose - when I have lost or won, this method signals the genetic object to adjust the fitness accordingly
// @ addThis - just directly add this to the fitnes
void chuck::winLose( int addThis ) {
	this->g.sendFit( addThis );
} // end winLose 

// alphaBeta - recursive method that returns minimax evalutions
// @ node - vector of currently evaluated board child 
// @ depth - how deep in the tree
// @ alpha - max value
// @ beta - min value
// @ moveOwner - currently evalutated player
double chuck::alphaBeta( vector<chuckChild> node, int depth, double alpha, double beta, owner moveOwner ) {
	double subTreeValue; // stores subTreeValue for max/min 
	vector<chuckChild> newNode; // used for spawning off children
	if ( depth <= 0 ) { // or is terminal, READ TODO
		vector<double> encodedInputs = this->encodeForNN( node );
		#ifdef DEBUGMODE
			for (int r=depth; r<=PLYDEPTH; r++)
				cout << "\t";
			double res = this->nn.infer( encodedInputs )[ 0 ];
			cout << res << endl;
			return res;
		#else
			return this->nn.infer( encodedInputs )[ 0 ];
		#endif
	}
	if ( moveOwner == this->whoiam ) { // its me, max player logic
		moveOwner = empty; // make it not me
		for ( int i = 0; i < 36 * 4; i++ ) { // for each spot on the board, times four, for each possible direction
			// current piece = i / 4 
			// current direction = i % 4
			if ( node[ i / 4 ] == meGood || node[ i / 4 ] == meBad ) { // its my piece, spawn off children
				subTreeValue = -20;
				if ( i % 4 == 0 && i / 4 > 5 && node[ i / 4 - 6 ] != meGood && node[ i / 4 - 6 ] != meBad ) { // spawn child up
					newNode = node;
					newNode[ i / 4 - 6 ] = newNode[ i / 4 ];
					newNode[ i / 4 ] = noGhost;
					#ifdef DEBUGMODE
						for (int r=depth; r<=PLYDEPTH; r++)
							cout << "\t";
						cout << "from " << i/ 4<< " to " << i/4 -6<< endl;
					#endif
					subTreeValue = this->alphaBeta( newNode, depth - 1, alpha, beta, moveOwner );
				} else if ( i % 4 == 1 && i / 4 < 30 && node[ i / 4 + 6 ] != meGood && node[ i / 4 + 6 ] != meBad ) { // spawn child down
					newNode = node;
					newNode[ i / 4 + 6 ] = newNode[ i / 4 ];
					newNode[ i / 4 ] = noGhost;
					#ifdef DEBUGMODE
						for (int r=depth; r<=PLYDEPTH; r++)
							cout << "\t";
						cout << "from " << i/ 4<< " to " << i/4 +6<< endl;
					#endif
					subTreeValue = this->alphaBeta( newNode, depth - 1, alpha, beta, moveOwner );
				} else if ( i % 4 == 2 && i / 4 % 6 != 0 && node[ i / 4 - 1 ] != meGood && node[ i / 4 - 1 ] != meBad ) { // spawn child left
					newNode = node;
					newNode[ i / 4 - 1 ] = newNode[ i / 4 ];
					newNode[ i / 4 ] = noGhost;
					#ifdef DEBUGMODE
						for (int r=depth; r<=PLYDEPTH; r++)
							cout << "\t";
						cout << "from " << i/ 4<< " to " << i/4 -1<< endl;
					#endif
					subTreeValue = this->alphaBeta( newNode, depth - 1, alpha, beta, moveOwner );
				} else if ( i % 4 == 3 && ( i / 4 + 1 ) % 6 != 0 && node[ i / 4 + 1 ] != meGood && node[ i / 4 + 1 ] != meBad ) { // spawn child right
					newNode = node;
					newNode[ i / 4 + 1 ] = newNode[ i / 4 ];
					newNode[ i / 4 ] = noGhost;
					#ifdef DEBUGMODE
						for (int r=depth; r<=PLYDEPTH; r++)
							cout << "\t";
						cout << "from " << i/ 4<< " to " << i/4 +1<< endl;
					#endif
					subTreeValue = this->alphaBeta( newNode, depth - 1, alpha, beta, moveOwner );
				}
				if ( subTreeValue > alpha )
					alpha = subTreeValue;
			} else { // its theirs or empty, skip a few iterations
				i += 3;
			}
			#ifdef PRUNING
				if ( beta <= alpha ) { 
					break; // beta cut off, prune rest of tree
				}
			#endif
		} 
		return alpha;
	} else { // its other player, min logic
		moveOwner = this->whoiam; // next move will be me
		for ( int i = 0; i < 36 * 4; i++ ) { // for each spot on the board, times four, for each possible direction
			// current piece = i / 4 
			// current direction = i % 4
			if ( node[ i / 4 ] == themGood || node[ i / 4 ] == themBad ) { // its other player's piece, spawn off children
				subTreeValue = 20;
				if ( i % 4 == 0 && i / 4 > 5 && node[ i / 4 - 6 ] != themBad && node[ i / 4 - 6 ] != themGood ) { // spawn child up
					newNode = node;
					newNode[ i / 4 - 6 ] = newNode[ i / 4 ];
					newNode[ i / 4 ] = noGhost;
					#ifdef DEBUGMODE
						for (int r=depth; r<=PLYDEPTH; r++)
							cout << "\t";
						cout << "from " << i/ 4<< " to " << i/4 -6<< endl;
					#endif
					subTreeValue = this->alphaBeta( newNode, depth - 1, alpha, beta, moveOwner );
				} else if ( i % 4 == 1 && i / 4 < 30 && node[ i / 4 + 6 ] != themBad && node[ i / 4 + 6 ] != themGood ) { // spawn child down
					newNode = node;
					newNode[ i / 4 + 6 ] = newNode[ i / 4 ];
					newNode[ i / 4 ] = noGhost;
					#ifdef DEBUGMODE
						for (int r=depth; r<=PLYDEPTH; r++)
							cout << "\t";
						cout << "from " << i/ 4<< " to " << i/4 +6<< endl;
					#endif
					subTreeValue = this->alphaBeta( newNode, depth - 1, alpha, beta, moveOwner );
				} else if ( i % 4 == 2 && i / 4 % 6 != 0 && node[ i / 4 - 1 ] != themBad && node[ i / 4 - 1 ] != themGood ) { // spawn child left
					newNode = node;
					newNode[ i / 4 - 1 ] = newNode[ i / 4 ];
					newNode[ i / 4 ] = noGhost;
					#ifdef DEBUGMODE
						for (int r=depth; r<=PLYDEPTH; r++)
							cout << "\t";
						cout << "from " << i/ 4<< " to " << i/4 -1<< endl;
					#endif
					subTreeValue = this->alphaBeta( newNode, depth - 1, alpha, beta, moveOwner );
				} else if ( i % 4 == 3 && ( i / 4 + 1 ) % 6 != 0 && node[ i / 4 + 1 ] != themBad && node[ i / 4 + 1 ] != themGood ) { // spawn child right
					newNode = node;
					newNode[ i / 4 + 1 ] = newNode[ i / 4 ];
					newNode[ i / 4 ] = noGhost;
					#ifdef DEBUGMODE
						for (int r=depth; r<=PLYDEPTH; r++)
							cout << "\t";
						cout << "from " << i/ 4<< " to " << i/4 +1<< endl;
					#endif
					subTreeValue = this->alphaBeta( newNode, depth - 1, alpha, beta, moveOwner );
				}
				if ( subTreeValue < beta )
					beta = subTreeValue;
			} else { // its mine or empty, skip a few iterations
				i += 3;
			}
			#ifdef PRUNING
				if ( beta <= alpha )
					break; // alpha cut off, prune rest of tree
			#endif
		} 
		return beta;
	}	
} // end alphaBeta

// encodeForNN - encodes a vector for input on the neuralNet
// @ toEncode - vector of chuckChild to encode
// PRE: toEncode must have a size of at least 40 to ensure that it has totals already calcutated
vector<double> chuck::encodeForNN( const vector<chuckChild> & toEncode ) {
	vector<double> output;
	int myGood = 0, myBad = 0, theirBad = 0, theirGood = 0;
	// add up all of each type
	for ( int i = 0; i < 36; i++ ) {
		if ( toEncode[ i ] == meGood ) 
			myGood++;
		else if ( toEncode[ i ] == meBad )
			myBad++;
		else if ( toEncode[ i ] == themBad )
			theirBad++;
		else if ( toEncode[ i ] == themGood )
			theirGood++;
	}
	// push output with proper number
	for ( int i = 0; i < 36; i++ ) {
		if ( toEncode[ i ] == meGood ) 
			output.push_back( this->curGoodValue[ myGood - 1 ] );
		else if ( toEncode[ i ] == meBad )
			output.push_back( this->curBadValue[ myBad - 1 ] );
		else if ( toEncode[ i ] == noGhost ) // empty
			output.push_back( 0 );
		else { // theirs
			if ( theirBad == 0 )
				theirBad++;
			if ( theirGood == 0 )
				theirGood++;
			output.push_back( this->curEneValue[ theirBad - 1 ][ theirGood - 1 ] );
		}
	}
	return output;
} // end encodeForNN

// writeOutGenes - tells the gene object to write out to file
void chuck::writeOutGenes() {
	this->g.writeToFile( GENFILE );
} // end writeOutGenes

// nextGene - tells the gene object to use the next gene, pushes new weights onto the neuralnetwork
void chuck::nextGene() {
	this->g.nextGene();
	vector<double> curChromo = this->g.getCurChromo(); // get current chromosome values
	this->nn.pushWeights( curChromo, NUMIGNOREWEIGHTS ); // put our currently being tested chromosome unto neural network
} // end nextGene

// useGene - tells the gene object to use a specified gene
void chuck::useGene( int gNum ) {
	this->g.useGene( gNum );
	vector<double> curChromo = this->g.getCurChromo(); // get current chromosome values
	this->nn.pushWeights( curChromo, NUMIGNOREWEIGHTS ); // put our currently being tested chromosome unto neural network
} // end useGene

// getGen - returns the gene object's current gen number
int chuck::getGen() {
	return this->g.getGen();
} // end getGen

// writeOutGood
// @param int wins number of wins
void chuck::writeOutGood( int wins ) {
	string f( GOODGENESFILE ); 
	this->g.writeOutCurrent( wins, f );
} // end writeOutGood

