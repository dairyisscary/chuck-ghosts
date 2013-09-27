// Eric Butler
// Ghosts! game genetic algorithm
// Based on the orignal code for CS340.
// Version: 2.2.0
// Started on: Tue Dec 20 13:42:45 EST 2011
// Copyright 2011

#include <stdio.h>
#include <stdlib.h>
#include <algorithm> // for sort
#include <fstream> // for file i/o
#include <iostream> // for buffer out
#include <string> // string
#include <time.h> // time for random
#include "genetics.h" 
using namespace std;

#define RANDDOUBLE ((double)rand()/(double)RAND_MAX)

// default cstor - fitness gets zero
chromosome::chromosome() 
	: fitness( 0 ) {
	// empty
} // end default cstor

// cstor
// @ w - genes to init with
// @ f - fitness to init with
chromosome::chromosome( vector<double> w, double f ) 
	: genes( w ), fitness( f ) {
	// empty
} // end cstor

// cstor 
// @ pSize - population size
// @ mutRate - mutation rate
// @ crossRate - crossover rate
// @ mxPt - maximum perturb for mutation
// @ cLen - length of each chromosome
genetics::genetics( int pSize, double mutRate, double crossRate, double mxPt, int cLen )
	: popSize( pSize ),
	mutationRate( mutRate ),
	crossoverRate( crossRate ),
	maxPerturb( mxPt ),
	chromoLength( cLen ),
	totalFit( 0 ),
	bestFitChromo( 0 ),
	bestFit( 0 ),
	worstFit( 99999999 ),
	avFit( 0 ) {
		srand( time( NULL ) ); // for randomness
} // end cstor

// dstor
genetics::~genetics() {
} // end dstor

// createRandom - create an init random generation of chromosomes
void genetics::createRandom() {
	this->gen = 0;
	this->curChromo = 0;
	this->population.clear(); // make sure this is cleared out of stuff
	for ( int i = 0; i < this->popSize; i++ ) {
		this->population.push_back( chromosome() );
		for ( int j = 0; j < this->chromoLength; j++ ) // for each gene
			this->population[ i ].genes.push_back( RANDDOUBLE - RANDDOUBLE );
	}
} // end createRandom

// writeToFile - write of genetics to a file
// @ file - file to write to
void genetics::writeToFile( string file ) {
	ofstream gfile;
	gfile.open( file.c_str() );
	gfile << this->gen << endl; // first generation
	gfile << this->curChromo; // first chromo
	for ( int i = 0; i < this->popSize; i++ ) {
		gfile << endl;
		for ( int j = 0; j < this->chromoLength; j++ )
			gfile << this->population[ i ].genes[ j ] << " ";
		gfile << this->population[ i ].fitness;
	}
	gfile.close();
} // end writeToFile

// readFromFile - read in genetics from a file
// @ file - file to read from
void genetics::readFromFile( string file ) {
	ifstream gfile;
	gfile.open( file.c_str() );
	double curGene = 0;
	gfile >> this->gen; // first line == currrent generation
	gfile >> this->curChromo; // third line == currently playing this chromo
	this->population.clear(); // clean out the population to ensure a freash start
	for ( int i = 0; i < this->popSize; i++ ) {
		this->population.push_back( chromosome() );
		for ( int j = 0; j < this->chromoLength; j++ ) {
			gfile >> curGene;
			this->population[ i ].genes.push_back( curGene );
		}
		gfile >> this->population[ i ].fitness;
	}
	gfile.close();
} // end readFromFile

// mutate - mutate a chromosome
// @ chromo - chromosome for mutation
void genetics::mutate( vector<double> & chromo ) {
	for ( int i = 0; i < chromo.size(); i++ ) {
		if ( ( (float) rand() / (float) RAND_MAX ) < this->mutationRate ) // change it?
			chromo[ i ] += ( RANDDOUBLE - RANDDOUBLE ) * this->maxPerturb;
	}
} // end mutate

// getChrmoRou - gets a chromosome from a roulette wheel, selection bias towards fitest individuals 
chromosome genetics::getChromoRou() {
	double slice = (double)( RANDDOUBLE * this->totalFit );
	double gFit = 0;
	for ( int i = 0 ; i < this->popSize; i++ ) {
		gFit += this->population[ i ].fitness;
		if ( gFit >= slice ) {
			return this->population[ i ];
		}
	}
	return this->population[ 0 ];
} // end getChromoRou
	
// crossover - crosses over two indivdual chromosomes
// @ mom - parent chromosome 1
// @ dad - parent chromosome 2
// @ baby1 - child chromosome 1
// @ baby2 - child chromosome 2
void genetics::crossover( const vector<double> & mom, const vector<double> & dad, vector<double> & baby1, vector<double> & baby2 ) {
	if ( ( RANDDOUBLE > this->crossoverRate ) || ( mom == dad ) ) { // dont bother if mom and dad are the same or we don't exceed our rate
		baby1 = mom;
		baby2 = dad;
		return;
	}
	int cp = rand() % ( this->chromoLength - 1 ); // cross over point between 0 and (chromolength - 2)
	int csp = rand() % ( this->chromoLength - cp - 1 ) + ( cp + 1 ); // cross over point between cp and chromolen
	//create the offspring
	int i;
	for ( i = 0; i < cp; i++ ){
		baby1.push_back( mom[ i ] );
		baby2.push_back( dad[ i ] );
	}
	for ( i = cp; i < csp; i++ ){
		baby1.push_back( dad[ i ] );
		baby2.push_back( mom[ i ] );
	}
	for ( i = csp; i < this->chromoLength; i++ ){
		baby1.push_back( mom[ i ] );
		baby2.push_back( dad[ i ] );
	}
} // end crossover

// evol - evolves the population of chrmosomes
vector<chromosome> genetics::evol() {
	this->reset();
	sort( this->population.begin(), this->population.end() ); // sort to make elite first
	this->calValues();
	vector<chromosome> newPop;
	this->getBest( 2, 1, newPop ); // param * param should be even so that roulette doesn't crash
	//repeat until a new population is generated
	while ( newPop.size() < this->popSize ) {
		chromosome mom = this->getChromoRou();
		chromosome dad = this->getChromoRou();
		vector<double> baby1;
		vector<double> baby2;
		this->crossover( mom.genes, dad.genes, baby1, baby2 );
		this->mutate( baby1 );
		this->mutate( baby2 );
		newPop.push_back( chromosome( baby1, 0 ) );
		newPop.push_back( chromosome( baby2, 0 ) );
	} // end while
	this->population = newPop;
	return newPop;
} // end evol

// calValues - calculate useful values for the current population
void genetics::calValues() {
	this->totalFit = 0;
	double high = -9999999999;
	double low = 9999999;
	for ( int i = 0 ; i < this->popSize; i++ ) {
		if ( this->population[ i ].fitness > high ) {
			high = this->population[ i ].fitness;
			this->bestFitChromo = i;
			this->bestFit = high;
		}
		if ( this->population[ i ].fitness < low ) {
			low = this->population[ i ].fitness;
			this->worstFit = low;
		}
		this->totalFit += this->population[ i ].fitness;
	}
	this->avFit = this->totalFit / this->popSize;
} // end calValue

// getBest - gets the best from the generation
// @ n - how many best to get
// @ numCopies - how many of each best chromosome to copy
// @ pop - referance to vector that these fit chromosomes will be put on
// PRE: the current population has been sorted
void genetics::getBest( int n, const int numCopies, vector<chromosome> & pop ) {
	while( n > 0 ) {
		for ( int i = 0; i < numCopies; i++ ) {
			pop.push_back( this->population[ (this->popSize - 1) - n ] );
			pop[ i ].fitness = 0;
		}
		n--;
	} // end while
} // end getBest

// reset - resets the useful values
void genetics::reset() {
	this->totalFit = 0;
	this->bestFit = 0;
	this->worstFit = 9999999;
	this->avFit = 0;
} // end reset

// getCurChromo - returns the chromosome of the currently being used individual
vector<double> & genetics::getCurChromo() {
	return this->population[ this->curChromo ].genes;
} // end getCurChromo

// getCurValue - returns a value from the currently being used individual
// @ where - index of value requested
double genetics::getCurValue( int where ) {
	return this->population[ this->curChromo ].genes[ where ];
} // end getCurEneValue

// sendFit - mutates the fitness level by addition
// @ addFitValue - value to add to the current fitness level
void genetics::sendFit( int addFitValue ) {
	this->population[ this->curChromo ].fitness += addFitValue;
} // end sendFitValue

// nextGene - iterates to the next gene, and evolves if need be
void genetics::nextGene() { 
	this->curChromo++;
	if ( this->curChromo == this->popSize ) { // end of generation
		this->curChromo = 0;
		this->gen++;
		this->evol();
	}
} // end nextGene

// useGene - sets curChormo equal to a specific number
void genetics::useGene( int gNum ) { 
	this->curChromo = gNum;
} // end nextGene

// getGen - returns the curent generation number
int genetics::getGen() {
	return this->gen;
} // end getGen

// writeOutCurrent
void genetics::writeOutCurrent( int wins, const string & file ) {
	ofstream gfile;
	gfile.open( file.c_str(), ios::out | ios::app ); // open for appending
	gfile << endl << endl;
	gfile << "Wins: " << wins << endl;
	for ( int j = 0; j < this->chromoLength; j++ )
		gfile << this->population[ this->curChromo ].genes[ j ] << " ";
	gfile << this->population[ this->curChromo ].fitness;
	gfile.close();
} // end writeOutCurrent

