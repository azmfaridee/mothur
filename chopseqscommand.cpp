/*
 *  chopseqscommand.cpp
 *  Mothur
 *
 *  Created by westcott on 5/10/10.
 *  Copyright 2010 Schloss Lab. All rights reserved.
 *
 */

#include "chopseqscommand.h"
#include "sequence.hpp"

//**********************************************************************************************************************

ChopSeqsCommand::ChopSeqsCommand(string option)  {
	try {
		abort = false;
		
		//allow user to run help
		if(option == "help") { help(); abort = true; }
		
		else {
			//valid paramters for this command
			string Array[] =  {"fasta","numbases","keep","outputdir","inputdir"};
			vector<string> myArray (Array, Array+(sizeof(Array)/sizeof(string)));
			
			OptionParser parser(option);
			map<string,string> parameters = parser.getParameters();
			
			ValidParameters validParameter;
			map<string,string>::iterator it;
			
			//check to make sure all parameters are valid for command
			for (map<string,string>::iterator it = parameters.begin(); it != parameters.end(); it++) { 
				if (validParameter.isValidParameter(it->first, myArray, it->second) != true) {  abort = true;  }
			}
			
			//if the user changes the output directory command factory will send this info to us in the output parameter 
			outputDir = validParameter.validFile(parameters, "outputdir", false);		if (outputDir == "not found"){	outputDir = "";		}
			
			//if the user changes the input directory command factory will send this info to us in the output parameter 
			string inputDir = validParameter.validFile(parameters, "inputdir", false);		
			if (inputDir == "not found"){	inputDir = "";		}
			else {
				string path;
				it = parameters.find("fasta");
				//user has given a template file
				if(it != parameters.end()){ 
					path = hasPath(it->second);
					//if the user has not given a path then, add inputdir. else leave path alone.
					if (path == "") {	parameters["fasta"] = inputDir + it->second;		}
				}
			}

			//check for required parameters
			fastafile = validParameter.validFile(parameters, "fasta", true);
			if (fastafile == "not open") { abort = true; }
			else if (fastafile == "not found") {  m->mothurOut("You must provide a fasta file."); m->mothurOutEndLine(); abort = true; }  	
			
			string temp = validParameter.validFile(parameters, "numbases", false);	if (temp == "not found") { temp = "0"; } 
			convert(temp, numbases);   
		
			keep = validParameter.validFile(parameters, "keep", false);		if (keep == "not found") { keep = "front"; } 
				
			if (numbases == 0)  { m->mothurOut("You must provide the number of bases you want to keep for the chops.seqs command."); m->mothurOutEndLine(); abort = true;  }
		}

	}
	catch(exception& e) {
		m->errorOut(e, "ChopSeqsCommand", "ChopSeqsCommand");
		exit(1);
	}
}
//**********************************************************************************************************************

void ChopSeqsCommand::help(){
	try {
		m->mothurOut("The chop.seqs command reads a fasta file and outputs a .chop.fasta containing the trimmed sequences.\n");
		m->mothurOut("The chop.seqs command parameters are fasta, numbases, and keep. fasta and numbases are required required.\n");
		m->mothurOut("The chop.seqs command should be in the following format: chop.seqs(fasta=yourFasta, numbases=yourNum, keep=yourKeep).\n");
		m->mothurOut("The numbases parameter allows you to specify the number of bases you want to keep.\n");
		m->mothurOut("The keep parameter allows you to specify whether you want to keep the front or the back of your sequence, default=front.\n");
		m->mothurOut("Example chop.seqs(fasta=amazon.fasta, numbases=200, keep=front).\n");
		m->mothurOut("Note: No spaces between parameter labels (i.e. fasta), '=' and parameters (i.e.yourFasta).\n\n");
	}
	catch(exception& e) {
		m->errorOut(e, "ChopSeqsCommand", "help");
		exit(1);
	}
}

//**********************************************************************************************************************

int ChopSeqsCommand::execute(){
	try {
		
		if (abort == true) { return 0; }
		
		string outputFileName = outputDir + getRootName(getSimpleName(fastafile)) + "chop.fasta";
		string outputFileNameAccnos = outputDir + getRootName(getSimpleName(fastafile)) + "chop.accnos";
		
		ofstream out;
		openOutputFile(outputFileName, out);
		
		ofstream outAcc;
		openOutputFile(outputFileNameAccnos, outAcc);
		
		ifstream in;
		openInputFile(fastafile, in);
		
		bool wroteAccnos = false;
		
		while (!in.eof()) {
			
			Sequence seq(in);
			
			if (m->control_pressed) { in.close(); out.close(); outAcc.close(); remove(outputFileName.c_str()); remove(outputFileNameAccnos.c_str()); return 0;  }
			
			if (seq.getName() != "") {
				string newSeqString = getChopped(seq);
				
				//output trimmed sequence
				if (newSeqString != "") {
					out << ">" << seq.getName() << endl << newSeqString << endl;
				}else{
					outAcc << seq.getName() << endl;
					wroteAccnos = true;
				}
			}
		}
		in.close();
		out.close();
		outAcc.close();
		
		m->mothurOutEndLine();
		m->mothurOut("Output File Name: "); m->mothurOutEndLine();
		m->mothurOut(outputFileName); m->mothurOutEndLine();	
		
		if (wroteAccnos) { m->mothurOut(outputFileNameAccnos); m->mothurOutEndLine();  }
		else {  remove(outputFileNameAccnos.c_str());  }
		
		m->mothurOutEndLine();
		
		return 0;		
	}

	catch(exception& e) {
		m->errorOut(e, "ChopSeqsCommand", "execute");
		exit(1);
	}
}
//**********************************************************************************************************************
string ChopSeqsCommand::getChopped(Sequence seq) {
	try {
		string temp = seq.getAligned();
		string tempUnaligned = seq.getUnaligned();
				
		//if needed trim sequence
		if (keep == "front") {//you want to keep the beginning
			int tempLength = tempUnaligned.length();

			if (tempLength > numbases) { //you have enough bases to remove some
				
				int stopSpot = 0;
				int numBasesCounted = 0;
				
				for (int i = 0; i < temp.length(); i++) {
					//eliminate N's
					if (toupper(temp[i]) == 'N') { 
						temp[i] == '.'; 
						tempLength--;
						if (tempLength < numbases) { stopSpot = 0; break; }
					}
					
					if(isalpha(temp[i])) { numBasesCounted++; }
					
					if (numBasesCounted >= numbases) { stopSpot = i; break; }
				}
				
				if (stopSpot == 0) { temp = ""; }
				else {  temp = temp.substr(0, stopSpot);  }
						
			}else { temp = ""; } //sequence too short
			
		}else { //you are keeping the back
			int tempLength = tempUnaligned.length();
			if (tempLength > numbases) { //you have enough bases to remove some
				
				int stopSpot = 0;
				int numBasesCounted = 0;
				
				for (int i = (temp.length()-1); i >= 0; i--) {
					//eliminate N's
					if (toupper(temp[i]) == 'N') { 
						temp[i] == '.'; 
						tempLength--;
						if (tempLength < numbases) { stopSpot = 0; break; }
					}
					
					if(isalpha(temp[i])) { numBasesCounted++; }

					if (numBasesCounted >= numbases) { stopSpot = i; break; }
				}
			
				if (stopSpot == 0) { temp = ""; }
				else {  temp = temp.substr(stopSpot+1);  }
			}
			else {  temp = ""; } //sequence too short
		}

		return temp;
	}
	catch(exception& e) {
		m->errorOut(e, "ChopSeqsCommand", "getChopped");
		exit(1);
	}
}
//**********************************************************************************************************************


