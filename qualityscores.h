#ifndef QUALITYSCORES
#define QUALITYSCORES

/*
 *  qualityscores.h
 *  Mothur
 *
 *  Created by Pat Schloss on 7/12/10.
 *  Copyright 2010 Schloss Lab. All rights reserved.
 *
 */

//DataStructure for a quality file.


#include "mothur.h"
#include "mothurout.h"
#include "sequence.hpp"

/**************************************************************************************************/

class QualityScores {
public:
	QualityScores();
	QualityScores(ifstream&);
	string getName();
	int getLength(){    return (int)qScores.size();  }
	vector<int> getQualityScores() { return qScores; }
	void printQScores(ofstream&);
	void trimQScores(int, int);
	void flipQScores();
	bool stripQualThreshold(Sequence&, double);
	bool stripQualRollingAverage(Sequence&, double);
	bool stripQualWindowAverage(Sequence&, int, int, double);
	bool cullQualAverage(Sequence&, double);
	void updateQScoreErrorMap(map<char, vector<int> >&, string, int, int, int);
	void updateForwardMap(vector<vector<int> >&, int, int, int);
	void updateReverseMap(vector<vector<int> >&, int, int, int);
    void setName(string n); 
    void setScores(vector<int> qs) { qScores = qs; seqLength = qScores.size(); }
    vector<int> getScores() { return qScores; }
	
private:
	
	double calculateAverage();
	MothurOut* m;
	vector<int> qScores;
	
	string seqName;
	int seqLength;
    
    string getSequenceName(ifstream&);
};
	
/**************************************************************************************************/

#endif
