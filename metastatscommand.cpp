/*
 *  metastatscommand.cpp
 *  Mothur
 *
 *  Created by westcott on 9/16/10.
 *  Copyright 2010 Schloss Lab. All rights reserved.
 *
 */

#include "metastatscommand.h"
#include "metastats.h"
#include "sharedutilities.h"

//**********************************************************************************************************************
vector<string> MetaStatsCommand::getValidParameters(){	
	try {
		string Array[] =  {"groups","label","outputdir","iters","threshold","g","design","sets","processors","inputdir"};
		vector<string> myArray (Array, Array+(sizeof(Array)/sizeof(string)));
		return myArray;
	}
	catch(exception& e) {
		m->errorOut(e, "MetaStatsCommand", "getValidParameters");
		exit(1);
	}
}
//**********************************************************************************************************************
MetaStatsCommand::MetaStatsCommand(){	
	try {
		abort = true;
		//initialize outputTypes
		vector<string> tempOutNames;
		outputTypes["metastats"] = tempOutNames;
	}
	catch(exception& e) {
		m->errorOut(e, "MetaStatsCommand", "MetaStatsCommand");
		exit(1);
	}
}
//**********************************************************************************************************************
vector<string> MetaStatsCommand::getRequiredParameters(){	
	try {
		string Array[] =  {"design"};
		vector<string> myArray (Array, Array+(sizeof(Array)/sizeof(string)));
		return myArray;
	}
	catch(exception& e) {
		m->errorOut(e, "MetaStatsCommand", "getRequiredParameters");
		exit(1);
	}
}
//**********************************************************************************************************************
vector<string> MetaStatsCommand::getRequiredFiles(){	
	try {
		string Array[] =  {"shared"};
		vector<string> myArray (Array, Array+(sizeof(Array)/sizeof(string)));
		return myArray;
	}
	catch(exception& e) {
		m->errorOut(e, "MetaStatsCommand", "getRequiredFiles");
		exit(1);
	}
}
//**********************************************************************************************************************

MetaStatsCommand::MetaStatsCommand(string option) {
	try {
		globaldata = GlobalData::getInstance();
		abort = false;
		allLines = 1;
		labels.clear();
		
		//allow user to run help
		if(option == "help") { help(); abort = true; }
		
		else {
			//valid paramters for this command
			string AlignArray[] =  {"groups","label","outputdir","iters","threshold","g","design","sets","processors","inputdir"};
			vector<string> myArray (AlignArray, AlignArray+(sizeof(AlignArray)/sizeof(string)));
			
			OptionParser parser(option);
			map<string,string> parameters = parser.getParameters();
			
			ValidParameters validParameter;
			
			//check to make sure all parameters are valid for command
			map<string,string>::iterator it;
			for (it = parameters.begin(); it != parameters.end(); it++) { 
				if (validParameter.isValidParameter(it->first, myArray, it->second) != true) {  abort = true;  }
			}
			
			//initialize outputTypes
			vector<string> tempOutNames;
			outputTypes["metastats"] = tempOutNames;
			
			//if the user changes the output directory command factory will send this info to us in the output parameter 
			outputDir = validParameter.validFile(parameters, "outputdir", false);		if (outputDir == "not found"){	
				outputDir = "";	
				outputDir += m->hasPath(globaldata->inputFileName); //if user entered a file with a path then preserve it	
			}
			
			//if the user changes the input directory command factory will send this info to us in the output parameter 
			string inputDir = validParameter.validFile(parameters, "inputdir", false);		
			if (inputDir == "not found"){	inputDir = "";		}
			else {
				string path;
				it = parameters.find("design");
				//user has given a template file
				if(it != parameters.end()){ 
					path = m->hasPath(it->second);
					//if the user has not given a path then, add inputdir. else leave path alone.
					if (path == "") {	parameters["design"] = inputDir + it->second;		}
				}
			}
			
			//check for required parameters
			designfile = validParameter.validFile(parameters, "design", true);
			if (designfile == "not open") { abort = true; }
			else if (designfile == "not found") {  designfile = "";  m->mothurOut("You must provide an design file."); m->mothurOutEndLine(); abort = true; }	
			
			//make sure the user has already run the read.otu command
			if ((globaldata->getSharedFile() == "")) {
				 m->mothurOut("You must read a list and a group, or a shared file before you can use the metastats command."); m->mothurOutEndLine(); abort = true; 
			}

			//check for optional parameter and set defaults
			// ...at some point should added some additional type checking...
			label = validParameter.validFile(parameters, "label", false);			
			if (label == "not found") { label = ""; }
			else { 
				if(label != "all") {  m->splitAtDash(label, labels);  allLines = 0;  }
				else { allLines = 1;  }
			}
			
			//if the user has not specified any labels use the ones from read.otu
			if (label == "") {  
				allLines = globaldata->allLines; 
				labels = globaldata->labels; 
			}
			
			groups = validParameter.validFile(parameters, "groups", false);			
			if (groups == "not found") { groups = ""; pickedGroups = false; }
			else { 
				pickedGroups = true;
				m->splitAtDash(groups, Groups);
				globaldata->Groups = Groups;
			}
			
			sets = validParameter.validFile(parameters, "sets", false);			
			if (sets == "not found") { sets = ""; }
			else { 
				m->splitAtDash(sets, Sets);
			}

			
			string temp = validParameter.validFile(parameters, "iters", false);			if (temp == "not found") { temp = "1000"; }
			convert(temp, iters); 
			
			temp = validParameter.validFile(parameters, "threshold", false);			if (temp == "not found") { temp = "0.05"; }
			convert(temp, threshold); 
			
			temp = validParameter.validFile(parameters, "processors", false);			if (temp == "not found"){	temp = "1";	}
			convert(temp, processors); 
		}

	}
	catch(exception& e) {
		m->errorOut(e, "MetaStatsCommand", "MetaStatsCommand");
		exit(1);
	}
}

//**********************************************************************************************************************

void MetaStatsCommand::help(){
	try {
		m->mothurOut("This command is based on the Metastats program, White, J.R., Nagarajan, N. & Pop, M. Statistical methods for detecting differentially abundant features in clinical metagenomic samples. PLoS Comput Biol 5, e1000352 (2009).\n");
		m->mothurOut("The metastats command can only be executed after a successful read.otu command of a list and group or shared file.\n");
		m->mothurOut("The metastats command outputs a .metastats file. \n");
		m->mothurOut("The metastats command parameters are iters, threshold, groups, label, design, sets and processors.  The design parameter is required.\n");
		m->mothurOut("The design parameter allows you to assign your groups to sets when you are running metastat. mothur will run all pairwise comparisons of the sets. It is required. \n");
		m->mothurOut("The design file looks like the group file.  It is a 2 column tab delimited file, where the first column is the group name and the second column is the set the group belongs to.\n");
		m->mothurOut("The sets parameter allows you to specify which of the sets in your designfile you would like to analyze. The set names are separated by dashes. THe default is all sets in the designfile.\n");
		m->mothurOut("The iters parameter allows you to set number of bootstrap permutations for estimating null distribution of t statistic.  The default is 1000. \n");
		m->mothurOut("The threshold parameter allows you to set the significance level to reject null hypotheses (default 0.05).\n");
		m->mothurOut("The groups parameter allows you to specify which of the groups in your groupfile you would like included. The group names are separated by dashes.\n");
		m->mothurOut("The label parameter allows you to select what distance levels you would like, and are also separated by dashes.\n");
		m->mothurOut("The processors parameter allows you to specify how many processors you would like to use.  The default is 1. \n");
		m->mothurOut("The metastats command should be in the following format: metastats(design=yourDesignFile).\n");
		m->mothurOut("Example metastats(design=temp.design, groups=A-B-C).\n");
		m->mothurOut("The default value for groups is all the groups in your groupfile, and all labels in your inputfile will be used.\n");
		m->mothurOut("Note: No spaces between parameter labels (i.e. groups), '=' and parameters (i.e.yourGroups).\n\n");

	}
	catch(exception& e) {
		m->errorOut(e, "MetaStatsCommand", "help");
		exit(1);
	}
}

//**********************************************************************************************************************

MetaStatsCommand::~MetaStatsCommand(){}

//**********************************************************************************************************************

int MetaStatsCommand::execute(){
	try {
	
		if (abort == true) { return 0; }
		
		designMap = new GroupMap(designfile);
		designMap->readDesignMap();
		
		read = new ReadOTUFile(globaldata->inputFileName);	
		read->read(&*globaldata); 
		input = globaldata->ginput;
		lookup = input->getSharedRAbundVectors();
		string lastLabel = lookup[0]->getLabel();
		
		//if the users enters label "0.06" and there is no "0.06" in their file use the next lowest label.
		set<string> processedLabels;
		set<string> userLabels = labels;
		
		//setup the pairwise comparions of sets for metastats
		//calculate number of comparisons i.e. with groups A,B,C = AB, AC, BC = 3;
		//make sure sets are all in designMap
		SharedUtil* util = new SharedUtil();  
		util->setGroups(Sets, designMap->namesOfGroups);  
		delete util;
		
		int numGroups = Sets.size();
		for (int a=0; a<numGroups; a++) { 
			for (int l = 0; l < a; l++) {	
				vector<string> groups; groups.push_back(Sets[a]); groups.push_back(Sets[l]);
				namesOfGroupCombos.push_back(groups);
			}
		}
	
		
		//only 1 combo
		if (numGroups == 2) { processors = 1; }
		else if (numGroups < 2)	{ m->mothurOut("Not enough sets, I need at least 2 valid sets. Unable to complete command."); m->mothurOutEndLine(); m->control_pressed = true; }
		
		#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux)
			if(processors != 1){
				int numPairs = namesOfGroupCombos.size();
				int numPairsPerProcessor = numPairs / processors;
			
				for (int i = 0; i < processors; i++) {
					int startPos = i * numPairsPerProcessor;
					if(i == processors - 1){
						numPairsPerProcessor = numPairs - i * numPairsPerProcessor;
					}
					lines.push_back(linePair(startPos, numPairsPerProcessor));
				}
			}
		#endif
		
		//as long as you are not at the end of the file or done wih the lines you want
		while((lookup[0] != NULL) && ((allLines == 1) || (userLabels.size() != 0))) {
			
			if (m->control_pressed) {  outputTypes.clear(); for (int i = 0; i < lookup.size(); i++) {  delete lookup[i];  } globaldata->Groups.clear(); delete read;  delete designMap;  for (int i = 0; i < outputNames.size(); i++) {	remove(outputNames[i].c_str()); } return 0; }
	
			if(allLines == 1 || labels.count(lookup[0]->getLabel()) == 1){			

				m->mothurOut(lookup[0]->getLabel()); m->mothurOutEndLine();
				process(lookup);
				
				processedLabels.insert(lookup[0]->getLabel());
				userLabels.erase(lookup[0]->getLabel());
			}
			
			if ((m->anyLabelsToProcess(lookup[0]->getLabel(), userLabels, "") == true) && (processedLabels.count(lastLabel) != 1)) {
				string saveLabel = lookup[0]->getLabel();
			
				for (int i = 0; i < lookup.size(); i++) {  delete lookup[i];  }  
				lookup = input->getSharedRAbundVectors(lastLabel);
				m->mothurOut(lookup[0]->getLabel()); m->mothurOutEndLine();
				
				process(lookup);
				
				processedLabels.insert(lookup[0]->getLabel());
				userLabels.erase(lookup[0]->getLabel());
				
				//restore real lastlabel to save below
				lookup[0]->setLabel(saveLabel);
			}
			
			lastLabel = lookup[0]->getLabel();
			//prevent memory leak
			for (int i = 0; i < lookup.size(); i++) {  delete lookup[i]; lookup[i] = NULL; }
			
			if (m->control_pressed) {  outputTypes.clear(); globaldata->Groups.clear(); delete read;  delete designMap;  for (int i = 0; i < outputNames.size(); i++) {	remove(outputNames[i].c_str()); } return 0; }

			//get next line to process
			lookup = input->getSharedRAbundVectors();				
		}
		
		if (m->control_pressed) { outputTypes.clear(); globaldata->Groups.clear(); delete read; delete designMap;  for (int i = 0; i < outputNames.size(); i++) {	remove(outputNames[i].c_str()); }  return 0; }

		//output error messages about any remaining user labels
		set<string>::iterator it;
		bool needToRun = false;
		for (it = userLabels.begin(); it != userLabels.end(); it++) {  
			m->mothurOut("Your file does not include the label " + *it); 
			if (processedLabels.count(lastLabel) != 1) {
				m->mothurOut(". I will use " + lastLabel + "."); m->mothurOutEndLine();
				needToRun = true;
			}else {
				m->mothurOut(". Please refer to " + lastLabel + "."); m->mothurOutEndLine();
			}
		}
	
		//run last label if you need to
		if (needToRun == true)  {
			for (int i = 0; i < lookup.size(); i++) { if (lookup[i] != NULL) { delete lookup[i]; } }  
			lookup = input->getSharedRAbundVectors(lastLabel);
			
			m->mothurOut(lookup[0]->getLabel()); m->mothurOutEndLine();
			
			process(lookup);
			
			for (int i = 0; i < lookup.size(); i++) {  delete lookup[i];  }
		}
	
		//reset groups parameter
		globaldata->Groups.clear();  
		delete input; globaldata->ginput = NULL;
		delete read;
		delete designMap;
		
		if (m->control_pressed) { outputTypes.clear(); for (int i = 0; i < outputNames.size(); i++) {	remove(outputNames[i].c_str()); } return 0;}
		
		m->mothurOutEndLine();
		m->mothurOut("Output File Names: "); m->mothurOutEndLine();
		for (int i = 0; i < outputNames.size(); i++) {	m->mothurOut(outputNames[i]); m->mothurOutEndLine();	}
		m->mothurOutEndLine();
		
		return 0;
	}
	catch(exception& e) {
		m->errorOut(e, "MetaStatsCommand", "execute");
		exit(1);
	}
}
//**********************************************************************************************************************

int MetaStatsCommand::process(vector<SharedRAbundVector*>& thisLookUp){
	try {
		if (pickedGroups) { eliminateZeroOTUS(thisLookUp); }
		
		#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux)
				if(processors == 1){
					driver(0, namesOfGroupCombos.size(), thisLookUp);
				}else{
					int process = 1;
					vector<int> processIDS;
		
					//loop through and create all the processes you want
					while (process != processors) {
						int pid = fork();
			
						if (pid > 0) {
							processIDS.push_back(pid);  //create map from line number to pid so you can append files in correct order later
							process++;
						}else if (pid == 0){
							driver(lines[process].start, lines[process].num, thisLookUp);
							exit(0);
						}else { m->mothurOut("unable to spawn the necessary processes."); m->mothurOutEndLine(); exit(0); }
					}
					
					//do my part
					driver(lines[0].start, lines[0].num, thisLookUp);
		
					//force parent to wait until all the processes are done
					for (int i=0;i<(processors-1);i++) { 
						int temp = processIDS[i];
						wait(&temp);
					}
				}
		#else
				driver(0, namesOfGroupCombos.size(), thisLookUp);
		#endif

		return 0;
		
	}
	catch(exception& e) {
		m->errorOut(e, "MetaStatsCommand", "process");
		exit(1);
	}
}
//**********************************************************************************************************************
int MetaStatsCommand::driver(int start, int num, vector<SharedRAbundVector*>& thisLookUp) { 
	try {
	
		//for each combo
		for (int c = start; c < (start+num); c++) {
			
			//get set names
			string setA = namesOfGroupCombos[c][0]; 
			string setB = namesOfGroupCombos[c][1];
			
			//get filename
			string outputFileName = outputDir +  m->getRootName(m->getSimpleName(globaldata->inputFileName)) + thisLookUp[0]->getLabel() + "." + setA + "-" + setB + ".metastats";
			outputNames.push_back(outputFileName); outputTypes["metastats"].push_back(outputFileName);
			int nameLength = outputFileName.length();
			char * output = new char[nameLength];
			strcpy(output, outputFileName.c_str());
	
			//build matrix from shared rabunds
			double** data;
			data = new double*[thisLookUp[0]->getNumBins()];
			
			vector<SharedRAbundVector*> subset;
			int setACount = 0;
			int setBCount = 0;
			for (int i = 0; i < thisLookUp.size(); i++) {
				string thisGroup = thisLookUp[i]->getGroup();
				
				//is this group for a set we want to compare??
				//sorting the sets by putting setB at the back and setA in the front
				if ((designMap->getGroup(thisGroup) == setB)) {  
					subset.push_back(thisLookUp[i]);
					setBCount++;
				}else if ((designMap->getGroup(thisGroup) == setA)) {
					subset.insert(subset.begin()+setACount, thisLookUp[i]);
					setACount++;
				}
			}
						
			if ((setACount == 0) || (setBCount == 0))  { 
				m->mothurOut("Missing shared info for " + setA + " or " + setB + ". Skipping comparison."); m->mothurOutEndLine(); 
				outputNames.pop_back();
			}else {
				//fill data
				for (int j = 0; j < thisLookUp[0]->getNumBins(); j++) {
					data[j] = new double[subset.size()];
					for (int i = 0; i < subset.size(); i++) {
						data[j][i] = (subset[i]->getAbundance(j));
					}
				}
				
				m->mothurOut("Comparing " + setA + " and " + setB + "..."); m->mothurOutEndLine(); 
				metastat_main(output, thisLookUp[0]->getNumBins(), subset.size(), threshold, iters, data, setACount);
				
				m->mothurOutEndLine(); 
			}
			
			//free memory
			delete output;
			for(int i = 0; i < thisLookUp[0]->getNumBins(); i++)  {  delete[] data[i];  }
			delete[] data; 
		}
		
		return 0;

	}
	catch(exception& e) {
		m->errorOut(e, "MetaStatsCommand", "driver");
		exit(1);
	}
}
//**********************************************************************************************************************
int MetaStatsCommand::eliminateZeroOTUS(vector<SharedRAbundVector*>& thislookup) {
	try {
		
		vector<SharedRAbundVector*> newLookup;
		for (int i = 0; i < thislookup.size(); i++) {
			SharedRAbundVector* temp = new SharedRAbundVector();
			temp->setLabel(thislookup[i]->getLabel());
			temp->setGroup(thislookup[i]->getGroup());
			newLookup.push_back(temp);
		}
		
		//for each bin
		for (int i = 0; i < thislookup[0]->getNumBins(); i++) {
			if (m->control_pressed) { for (int j = 0; j < newLookup.size(); j++) {  delete newLookup[j];  } return 0; }
		
			//look at each sharedRabund and make sure they are not all zero
			bool allZero = true;
			for (int j = 0; j < thislookup.size(); j++) {
				if (thislookup[j]->getAbundance(i) != 0) { allZero = false;  break;  }
			}
			
			//if they are not all zero add this bin
			if (!allZero) {
				for (int j = 0; j < thislookup.size(); j++) {
					newLookup[j]->push_back(thislookup[j]->getAbundance(i), thislookup[j]->getGroup());
				}
			}
		}

		for (int j = 0; j < thislookup.size(); j++) {  delete thislookup[j];  }

		thislookup = newLookup;
		
		return 0;
 
	}
	catch(exception& e) {
		m->errorOut(e, "MetaStatsCommand", "eliminateZeroOTUS");
		exit(1);
	}
}
//**********************************************************************************************************************