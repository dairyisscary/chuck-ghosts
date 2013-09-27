// Eric Butler
// Ghosts! game player
// Based on the orignal code for CS340.
// Version: 2.2.0
// Started on: Tue Dec 20 13:42:45 EST 2011
// Copyright 2011

#include <iostream>
#ifndef GENETICS_H
#define GENETICS_H

#include <vector>
#include <string>
using namespace std;

struct chromosome {
	vector<double> genes; // (weights)
	int fitness;
	chromosome();
	chromosome( vector<double>, double );

	// overload '<' operator used for sorting
	friend bool operator<( const chromosome & lhs, const chromosome & rhs ) {
		return ( lhs.fitness < rhs.fitness );
	}
};

class genetics {

	private:
		vector<chromosome> population;
		int popSize;
		int chromoLength;
		double totalFit;
		double bestFit;
		double avFit;
		double worstFit;
		int bestFitChromo;
		double mutationRate; 
		double crossoverRate; 
		double maxPerturb; 
		int gen;
		int curChromo;
		void crossover( const vector<double> &, const vector<double> &, vector<double> &, vector<double> & );
		void mutate( vector<double> & );
		chromosome getChromoRou();
		void getBest( int, const int, vector<chromosome> & );
		void calValues();
		void reset();
	
	public:
		genetics( int, double, double, double, int );
		~genetics();
		void createRandom();
		void writeToFile( string );
		void readFromFile( string );
		vector<chromosome> evol();
		vector<double> & getCurChromo();
		void sendFit( int );
		double getCurValue( int );
		void nextGene();
		void useGene( int );
		int getGen();
		void writeOutCurrent( int, const string & );

};

#endif

