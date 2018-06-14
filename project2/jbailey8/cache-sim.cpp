#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <math.h>

struct instruction{
	unsigned long long address;
	char LS;
};

std::ifstream input;
std::ofstream output;
unsigned long long totalInstructions = 0;
std::vector<struct instruction> instructions;

void Direct_Mapped(){
	int hits = 0;
	int cacheSize = 1024;
	int cacheLine = 32;
	int tag, index, tableSize;
	while(cacheSize <= 32768){
		tableSize = cacheSize / cacheLine;
		int cacheTable[tableSize];
		for(int i = 0; i < instructions.size(); i++){
			index = (instructions[i].address / cacheLine) % tableSize;
			tag = instructions[i].address / cacheSize;
			if(cacheTable[index] == tag) hits++;
			else cacheTable[index] = tag;
		}

		if(cacheSize != 32768) output << hits << ',' << totalInstructions << "; ";
		else{
			output << hits << ',' << totalInstructions << ';';
		}
		if(cacheSize != 16384) cacheSize = cacheSize * 4;
		else cacheSize = cacheSize * 2;
		hits = 0;
	}

	output << std::endl;
	
	return;
}

void Set_Associative(){
	int hits = 0;
	int cacheLine = 32;
	int cacheSize = 16384;
	int associativity = 2;
	int timeStamp = totalInstructions;
	int tag, index, tableSize;
	bool found = false;
	while(associativity <= 16){
		tableSize = cacheSize/cacheLine/associativity;
		int **cacheTable = new int*[tableSize];
		int **timeStampTable = new int*[tableSize];
		for(int i = 0; i < tableSize; i++){
			cacheTable[i] = new int[associativity];
			timeStampTable[i] = new int[associativity];
			//This took me 4 days to realise the 2-D array isn't created with all it's values set to 0.
			for(int j = 0; j < associativity; j++){
				cacheTable[i][j] = instructions[i].address;
				timeStampTable[i][j] = 0;
			}
		}
		for(int i = 0; i < instructions.size(); i++){
			timeStamp++;
			index = (instructions[i].address / cacheLine) % tableSize;
			tag = instructions[i].address / (cacheSize/associativity);
			found = false;
			for(int j = 0; j < associativity; j++){
				if(cacheTable[index][j] == tag){
					hits++;
					timeStampTable[index][j] = timeStamp;//Most recently used, largest timestamp
					found = true;
					break;
				}
			}
			if(!found){
				int smallest = timeStampTable[index][0];
				int updateIndex = 0;
				for(int j = 0; j < associativity; j++){
					if(timeStampTable[index][j] < smallest){//Looking for least recently used slot
						smallest = timeStampTable[index][j];
						updateIndex = j;
					}
				}
				cacheTable[index][updateIndex] = tag;
				timeStampTable[index][updateIndex] = timeStamp;
			}
		}
		timeStamp = 0;
		if(associativity != 16) output << hits << ',' << totalInstructions << "; ";
		else output << hits << ',' << totalInstructions << ';';
		associativity = associativity * 2;
		hits = 0;
		//Memory Leaks
		for(int i = 0; i < tableSize; i++){
			delete[] cacheTable[i];
			delete[] timeStampTable[i];
		}
		delete[] cacheTable;
		delete[] timeStampTable;
	}

	output << std::endl;

	return;
}

void Fully_Associative_LRU(){
	int hits = 0;
	int cacheLine = 32;
	int cacheSize = 16384;
	int timeStamp = 0;
	int tag;
	int tableSize = cacheSize/cacheLine;
	int cacheTable[tableSize];
	int timeStampTable[tableSize];
	for(int i = 0; i < tableSize; i++) timeStampTable[i] = 0;
	bool found = false;
	for(int i = 0; i < instructions.size(); i++){
		timeStamp++;
		tag = instructions[i].address/cacheLine;
		found = false;
		for(int j = 0; j < tableSize; j++){
			if(cacheTable[j] == tag){
				hits++;
				timeStampTable[j] = timeStamp;
				found = true;
				break;
			}
		}
		if(!found){
			int smallest = instructions.size();
			int updateIndex = 0;
			for(int j = 0; j < tableSize; j++){
				if(timeStampTable[j] < smallest){//Looking for least recently used slot
					smallest = timeStampTable[j];
					updateIndex = j;
				}
			}
			cacheTable[updateIndex] = tag;
			timeStampTable[updateIndex] = timeStamp;
		}
	}
	timeStamp = 0;
	output << hits << ',' << totalInstructions << ';';
	hits = 0;

	output << std::endl;

	
	return;
}

/*To do*/
void Fully_Associative_HC(){
	int hits = 0;
	int cacheLine = 32;
	int cacheSize = 16384;
	int tag;
	int tableSize = cacheSize/cacheLine;
	int cacheTable[tableSize];
	int hotCold[tableSize];
	for(int i = 0; i < tableSize; i++){
		cacheTable[i] = 0;
		hotCold[i] = 0;
	}
	for(int i = 0; i < instructions.size(); i++){
		tag = instructions[i].address/cacheLine;
		bool found = false;
		for(int j = 0; j < tableSize; j++){
			if(cacheTable[j] == tag){
				hits++;
				int parent = 0;
				for(int k = 0; k < log2(tableSize); k++){
					int update = (j / (tableSize / (2 << k))) % 2;
					//std::cout << update << std::endl;
					//sleep(1);
					if(update == 0){
						if(hotCold[parent] == update) hotCold[parent] = 1;
						parent = 2 * parent + 1;
					}
					else{
						if(hotCold[parent] == update) hotCold[parent] = 0;
						parent = 2 * (parent + 1);
					}
				}
				//std::cout << "BYE" << std::endl;
				//sleep(1);
				found = true;
				break;
			}
		}
		int victim = 0;
		int updateIndex = 0;
		if(!found){
			for(int j = 0; j < log2(tableSize); j++){
				if(hotCold[updateIndex] == 0){
					hotCold[updateIndex] = 1;
					updateIndex = 2 * updateIndex + 1;
				}
				else{
					hotCold[updateIndex] = 0;
					updateIndex = 2 * (updateIndex + 1);
				}
			}
			//std::cout << updateIndex << std::endl;
			//sleep(2);
			victim = updateIndex - (tableSize-1);
			cacheTable[victim] = tag;
		}

	}

	output << hits << ',' << totalInstructions << ';';
	hits = 0;

	output << std::endl;

	
	return;
}

void Set_Associative_Write_Miss(){
	int hits = 0;
	int cacheLine = 32;
	int cacheSize = 16384;
	int associativity = 2;
	int timeStamp = totalInstructions;
	int tag, index, tableSize;
	bool found = false;
	while(associativity <= 16){
		tableSize = cacheSize/cacheLine/associativity;
		int **cacheTable = new int*[tableSize];
		int **timeStampTable = new int*[tableSize];
		for(int i = 0; i < tableSize; i++){
			cacheTable[i] = new int[associativity];
			timeStampTable[i] = new int[associativity];
			//This took me 4 days to realise the 2-D array isn't created with all it's values set to 0.
			//I'm fairly mad
			for(int j = 0; j < associativity; j++) {
				cacheTable[i][j] = instructions[i].address;
				timeStampTable[i][j] = 0;
			}
		}
		for(int i = 0; i < instructions.size(); i++){
			timeStamp++;
			index = (instructions[i].address / cacheLine) % tableSize;
			tag = instructions[i].address / (cacheSize/associativity);
			found = false;
			for(int j = 0; j < associativity; j++){
				if(cacheTable[index][j] == tag){
					hits++;
					timeStampTable[index][j] = timeStamp;//Most recently used, largest timestamp
					found = true;
					break;
				}
			}
			if(!found){
				if(instructions[i].LS != 'S'){
					int smallest = timeStampTable[index][0];
					int updateIndex = 0;
					for(int j = 0; j < associativity; j++){
						if(timeStampTable[index][j] < smallest){//Looking for least recently used slot
							smallest = timeStampTable[index][j];
							updateIndex = j;
						}
					}
					cacheTable[index][updateIndex] = tag;
					timeStampTable[index][updateIndex] = timeStamp;
				}
			}
		}
		timeStamp = 0;
		if(associativity != 16) output << hits << ',' << totalInstructions << "; ";
		else output << hits << ',' << totalInstructions << ';';
		associativity = associativity * 2;
		hits = 0;
		//Memory Leaks
		for(int i = 0; i < tableSize; i++){
			delete[] cacheTable[i];
			delete[] timeStampTable[i];
		}
		delete[] cacheTable;
		delete[] timeStampTable;
	}

	output << std::endl;

	return;
}

void Set_Associative_Prefetching(){
	int hits = 0;
	int cacheLine = 32;
	int cacheSize = 16384;
	int associativity = 2;
	int timeStamp = totalInstructions;
	int tag, index, tableSize;
	int prefetchTag, prefetchIndex;
	bool found = false;
	bool prefetchFound = false;
	while(associativity <= 16){
		tableSize = cacheSize/cacheLine/associativity;
		int **cacheTable = new int*[tableSize];
		int **timeStampTable = new int*[tableSize];
		for(int i = 0; i < tableSize; i++){
			cacheTable[i] = new int[associativity];
			timeStampTable[i] = new int[associativity];
			//This took me 4 days to realise the 2-D array isn't created with all it's values set to 0.
			for(int j = 0; j < associativity; j++){
				cacheTable[i][j] = instructions[i].address;
				timeStampTable[i][j] = 0;
			}
		}
		for(int i = 0; i < instructions.size(); i++){
			timeStamp++;
			index = (instructions[i].address / cacheLine) % tableSize;
			tag = instructions[i].address / (cacheSize/associativity);
			prefetchIndex = (index + 1) % tableSize;
			prefetchTag = ((instructions[i].address + cacheLine) / (cacheSize/associativity));
			found = false;
			prefetchFound = false;
			for(int j = 0; j < associativity; j++){
				if(cacheTable[index][j] == tag){
					hits++;
					timeStampTable[index][j] = timeStamp;//Most recently used, largest timestamp
					found = true;
					break;
				}
			}
			for(int j = 0; j < associativity; j++){
				if(cacheTable[prefetchIndex][j] == prefetchTag){
					timeStampTable[prefetchIndex][j] = timeStamp;
					prefetchFound = true;
					break;
				}
			}
			if(!found){
				int smallest = timeStampTable[index][0];
				int updateIndex = 0;
				for(int j = 0; j < associativity; j++){
					if(timeStampTable[index][j] < smallest){//Looking for least recently used slot
						smallest = timeStampTable[index][j];
						updateIndex = j;
					}
				}
				cacheTable[index][updateIndex] = tag;
				timeStampTable[index][updateIndex] = timeStamp;
			}
			if(!prefetchFound){
				int smallest = timeStampTable[prefetchIndex][0];
				int updateIndex = 0;
				for(int j = 0; j < associativity; j++){
					if(timeStampTable[prefetchIndex][j] < smallest){
						smallest = timeStampTable[prefetchIndex][j];
						updateIndex = j;
					}
				}
				cacheTable[prefetchIndex][updateIndex] = prefetchTag;
				timeStampTable[prefetchIndex][updateIndex] = timeStamp;
			}
		}
		timeStamp = 0;
		if(associativity != 16) output << hits << ',' << totalInstructions << "; ";
		else output << hits << ',' << totalInstructions << ';';
		associativity = associativity * 2;
		hits = 0;
		//Memory Leaks
		for(int i = 0; i < tableSize; i++){
			delete[] cacheTable[i];
			delete[] timeStampTable[i];
		}
		delete[] cacheTable;
		delete[] timeStampTable;
	}

	output << std::endl;

	return;
}

void Prefetch_Miss(){
	int hits = 0;
	int cacheLine = 32;
	int cacheSize = 16384;
	int associativity = 2;
	int timeStamp = totalInstructions;
	int tag, index, tableSize;
	int prefetchTag, prefetchIndex;
	bool found = false;
	bool prefetchFound = false;
	while(associativity <= 16){
		tableSize = cacheSize/cacheLine/associativity;
		int **cacheTable = new int*[tableSize];
		int **timeStampTable = new int*[tableSize];
		for(int i = 0; i < tableSize; i++){
			cacheTable[i] = new int[associativity];
			timeStampTable[i] = new int[associativity];
			//This took me 4 days to realise the 2-D array isn't created with all it's values set to 0.
			for(int j = 0; j < associativity; j++){
				cacheTable[i][j] = instructions[i].address;
				timeStampTable[i][j] = 0;
			}
		}
		for(int i = 0; i < instructions.size(); i++){
			timeStamp++;
			index = (instructions[i].address / cacheLine) % tableSize;
			tag = instructions[i].address / (cacheSize/associativity);
			prefetchIndex = (index + 1) % tableSize;
			prefetchTag = ((instructions[i].address + cacheLine) / (cacheSize/associativity));
			found = false;
			prefetchFound = false;
			for(int j = 0; j < associativity; j++){
				if(cacheTable[index][j] == tag){
					hits++;
					timeStampTable[index][j] = timeStamp;//Most recently used, largest timestamp
					found = true;
					break;
				}
			}
			if(!found){
				for(int j = 0; j < associativity; j++){
					if(cacheTable[prefetchIndex][j] == prefetchTag){
						timeStampTable[prefetchIndex][j] = timeStamp;
						prefetchFound = true;
						break;
					}
				}
				int smallest = timeStampTable[index][0];
				int updateIndex = 0;
				for(int j = 0; j < associativity; j++){
					if(timeStampTable[index][j] < smallest){//Looking for least recently used slot
						smallest = timeStampTable[index][j];
						updateIndex = j;
					}
				}
				cacheTable[index][updateIndex] = tag;
				timeStampTable[index][updateIndex] = timeStamp;
			}
			if(!prefetchFound && !found){
				int smallest = timeStampTable[prefetchIndex][0];
				int updateIndex = 0;
				for(int j = 0; j < associativity; j++){
					if(timeStampTable[prefetchIndex][j] < smallest){
						smallest = timeStampTable[prefetchIndex][j];
						updateIndex = j;
					}
				}
				cacheTable[prefetchIndex][updateIndex] = prefetchTag;
				timeStampTable[prefetchIndex][updateIndex] = timeStamp;
			}
		}
		timeStamp = 0;
		if(associativity != 16) output << hits << ',' << totalInstructions << "; ";
		else output << hits << ',' << totalInstructions << ';';
		associativity = associativity * 2;
		hits = 0;
		//Memory Leaks
		for(int i = 0; i < tableSize; i++){
			delete[] cacheTable[i];
			delete[] timeStampTable[i];
		}
		delete[] cacheTable;
		delete[] timeStampTable;
	}

	output << std::endl;

	return;
}

int main(int argc, char * argv[]){
	char behavior;
	unsigned long long address;
	struct instruction instr;
	input.open(argv[1]);
	output.open(argv[2]);

	while(input >> behavior >> std::hex >> address){
		instr.LS = behavior;
		instr.address = address;
		instructions.push_back(instr);
		totalInstructions++;
	}

	Direct_Mapped();
	Set_Associative();
	Fully_Associative_LRU();
	Fully_Associative_HC();
	Set_Associative_Write_Miss();
	Set_Associative_Prefetching();
	Prefetch_Miss();

	return 0;
}
