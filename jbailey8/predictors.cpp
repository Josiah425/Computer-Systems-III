#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

struct branch{
	unsigned long long address;
	int taken;
} branch;

int main(int argc, char * argv[]){
	std::string behavior;
	unsigned long long int address;
	unsigned long long int totalBranches = 0;
	unsigned long long int correct = 0;
	unsigned long long int i = 0;
	unsigned long long int j = 0;
	int mod = 8;
	int global = 0;
	int tableSize = 7;
	int tableSizes[tableSize] = {16, 32, 128, 256, 512, 1024, 2048};
	bool gRight = false;
	bool bRight = false;
	std::vector<struct branch> branches;
	struct branch cBranch;
	std::ifstream iFile;
	iFile.open(argv[1]);
	std::ofstream oFile;
	oFile.open(argv[2]);

	while(iFile >> std::hex >> address >> behavior){
		cBranch.address = address;
		if(behavior == "T") cBranch.taken = 1;
		else cBranch.taken = 0;
		branches.push_back(cBranch);
		totalBranches++;
	}

	//All Taken && All Not Taken
	for(i = 0; i < branches.size(); i++){
		if(branches[i].taken == 1) correct++;
	}
	oFile << correct << ',' << totalBranches << ';' << std::endl;
	oFile << (totalBranches - correct) << ',' << totalBranches << ';' << std::endl;

	correct = 0;
	
	//Bimodal Single Bit
	for(i = 0; i < tableSize; i++){
		int * Prediction = new int[tableSizes[i]]; //this initializes an array of size specified in table
		//Directions say to initialize prediction table to taken 1 stands for taken
		for(j = 0; j < tableSizes[i]; j++){
			Prediction[j] = 1;
		}
		for(j = 0; j < branches.size(); j++){
			int lookUp = branches[j].address % tableSizes[i];
			if(Prediction[lookUp] == branches[j].taken) correct++;
			else {
				//Switch values in prediction table
				if(Prediction[lookUp] == 1) Prediction[lookUp] = 0;
				else Prediction[lookUp] = 1;
			}
		}
		//Only doing this to get rid of the space at the end of the last tablesize
		if(i != 6) oFile << correct << ',' << totalBranches << "; ";
		else oFile << correct << ',' << totalBranches << ';' << std::endl;
		delete(Prediction);
		correct = 0;
	}

	//Bimodal Double bit
	for(i = 0; i < tableSize; i++){
		int * Prediction = new int[tableSizes[i]];
		//Directions say to initialize Prediction table to "Strongly Taken" 11 (3) stands for Strongly Taken
		for(j = 0; j < tableSizes[i]; j++){
			Prediction[j] = 3;
		}
		for(j = 0; j < branches.size(); j++){
			int lookUp = branches[j].address % tableSizes[i];
			if((Prediction[lookUp] == 3 || Prediction[lookUp] == 2) && branches[j].taken == 1) {
				correct++;
				Prediction[lookUp] = 3;
			}
			else if((Prediction[lookUp] == 1 || Prediction[lookUp] == 0) && branches[j].taken == 0){
				correct++;
				Prediction[lookUp] = 0;
			}
			else if(Prediction[lookUp] == 3 && branches[j].taken == 0){
				Prediction[lookUp] = 2;
			}
			else if(Prediction[lookUp] == 2 && branches[j].taken == 0){
				Prediction[lookUp] = 1;
			}
			else if(Prediction[lookUp] == 1 && branches[j].taken == 1){
				Prediction[lookUp] = 2;
			}
			else if(Prediction[lookUp] == 0 && branches[j].taken == 1){
				Prediction[lookUp] = 1;
			}
		}
		if(i != 6) oFile << correct << ',' << totalBranches << "; ";
		else oFile << correct << ',' << totalBranches << ';' << std::endl;
		correct = 0;
		delete(Prediction);
	}

	//GShare
	for(i = 3; i <= 11; i++){
		int Prediction[2048];
		//Default is Strongly Taken 11 (3)
		for(j = 0; j < 2048; j++){
			Prediction[j] = 3;
		}
		for(j = 0; j < branches.size(); j++){
			//XOR PC with global history, global obviously starts at 0 (^ = XOR)
			int lookUp = (branches[j].address % 2048) ^ global;
			if(branches[j].taken == 1){
				global = (global << 1) % mod + 1; //shifts bits over by 1, and places 1 in least significant bit
				if(Prediction[lookUp] == 3){
					correct++;
				}
				else if(Prediction[lookUp] == 2){
					correct++;
					Prediction[lookUp] = 3;
				}
				else if(Prediction[lookUp] == 1){
					Prediction[lookUp] = 2;
				}
				else{
					Prediction[lookUp] = 1;
				}
			}
			else{
				global = (global << 1) % mod; //shifts bits over by 1, and places 0 in least significant bit
				if(Prediction[lookUp] == 3){
					Prediction[lookUp] = 2;
				}
				else if(Prediction[lookUp] == 2){
					Prediction[lookUp] = 1;
				}
				else if(Prediction[lookUp] == 1){
					correct++;
					Prediction[lookUp] = 0;
				}
				else{
					correct++;
				}
			}
		}
		if(mod != 2048) oFile << correct << ',' << totalBranches << "; ";
		else oFile << correct << ',' << totalBranches << ';' << std::endl;
		correct = 0;
		mod = mod << 1; //Used to update how many bits we are working with in global history register
	}

	//Tournament
	//00 GShare, 01 WGShare, 10 WBimodal, 11 Bimodal
	global = 0;
	mod = 2048;
	int Prediction[2048][3]; //first is GShare, second is bimodal, third is selector
	for(int j = 0; j < 2048; j++){
		Prediction[j][0] = 3;
		Prediction[j][1] = 3;
		Prediction[j][2] = 0;
	}

	for(j = 0; j < branches.size(); j++){
		int gLookUp = (branches[j].address % mod) ^ global;
		int bLookUp = branches[j].address % mod;
		//GShare 0
		if(branches[j].taken == 1){
			global = (global << 1) % mod + 1;
			if(Prediction[gLookUp][0] == 3) gRight = true;
			else if(Prediction[gLookUp][0] == 2){
				gRight = true;
				Prediction[gLookUp][0] = 3;
			}
			else if(Prediction[gLookUp][0] == 1) Prediction[gLookUp][0] = 2;
			else Prediction[gLookUp][0] = 1;
		}
		else{
			global = (global << 1) % mod;
			if(Prediction[gLookUp][0] == 3) Prediction[gLookUp][0] = 2;
			else if(Prediction[gLookUp][0] == 2) Prediction[gLookUp][0] = 1;
			else if(Prediction[gLookUp][0] == 1) {
				gRight = true;
				Prediction[gLookUp][0] = 0;
			}
			else gRight = true;
		}
		//Bimodal 1
		if(branches[j].taken == 1){
			if(Prediction[bLookUp][1] == 3) bRight = true;
			else if(Prediction[bLookUp][1] == 2){
				bRight = true;
				Prediction[bLookUp][1] = 3;
			}
			else if(Prediction[bLookUp][1] == 1) Prediction[bLookUp][1] = 2;
			else Prediction[bLookUp][1] = 1;
		}
		else{
			if(Prediction[bLookUp][1] == 3) Prediction[bLookUp][1] = 2;
			else if(Prediction[bLookUp][1] == 2) Prediction[bLookUp][1] = 1;
			else if(Prediction[bLookUp][1] == 1){
				bRight = true;
				Prediction[bLookUp][1] = 0;
			}
			else bRight = true;
		}
		//Tournament selector update 2
		if(bRight == true && gRight == true) correct++;
		else if(bRight == true){
			if(Prediction[bLookUp][2] == 0) Prediction[bLookUp][2] = 1;
			else if(Prediction[bLookUp][2] == 1) Prediction[bLookUp][2] = 2;
			else if(Prediction[bLookUp][2] == 2){
				Prediction[bLookUp][2] = 3;
				correct++;
			}
			else correct++;
		}
		else if(gRight == true){
			if(Prediction[bLookUp][2] == 0) correct++;
			else if(Prediction[bLookUp][2] == 1){
				correct++;
				Prediction[bLookUp][2] = 0;
			}
			else if(Prediction[bLookUp][2] == 2) Prediction[bLookUp][2] = 1;
			else Prediction[bLookUp][2] = 2;
		}
		bRight = false;
		gRight = false;
	}
	
	oFile << correct << ',' << totalBranches << ';' << std::endl;

	iFile.close();
	oFile.close();

	return 0;
}
