// Eric Butler
// Ghosts! game player
// Based on the orignal code for CS340.
// Version: 2.2.0
// Started on: Tue Dec 20 13:42:45 EST 2011
// Copyright 2011

#ifndef chuck_h
#define chuck_h

#include <iostream>
#include "board.h"
#include "player.h"
#include "neuralnet.h"
#include "genetics.h"

enum chuckChild { noGhost, meGood, meBad, themGood, themBad };

class chuck : public player {

	private:
		neuralnet nn;
		genetics g;
		double curBadValue[ 4 ];
		double curGoodValue[ 4 ];
		double curEneValue[ 4 ][ 4 ];
		double alphaBeta( vector<chuckChild>, int, double, double, owner );
		vector<double> encodeForNN( const vector<chuckChild> & );

	public:
		owner whoiam;
		chuck();
		chuck( owner );
		~chuck();
		virtual void setup( ghostboard & );
		virtual void move( ghostboard, int &, int &, int &, int & );
		void winLose( owner );
		void winLose( int );
		void writeOutGenes();
		void nextGene();
		void useGene( int );
		int getGen();
		void writeOutGood( int );

};

#endif

