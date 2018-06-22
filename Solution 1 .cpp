/** Commands

g++ -fopenmp -std=c++11 alternateImplementation.cpp
./a.out 4

**/

#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bits/stdc++.h>

//For using directory calls
#include <dirent.h>

//For using stat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

#define MAX_BLOCK_SIZE 4096

//Function declaration
void deriveFreq(vector<string> files[], string dirName)
{
	//Open the direc28tory
	DIR* ds = opendir(dirName.c_str());
	struct dirent* tempDir; 
	struct stat buf;
	while((tempDir = readdir(ds)) != NULL)
	{
		//Check if directories are not current or parent
		if((strcmp(tempDir->d_name,".") != 0) && (strcmp(tempDir->d_name,"..") != 0))
		{
			string str(tempDir->d_name);

			//Copy file stat info in buffer
		    stat((dirName+"/"+str).c_str(), &buf);

		    //Check if the file is a directory
			if(S_ISDIR(buf.st_mode))
			{
				// printf("dirname + tempDir->d_name %s\n\n",(dirName+"/"+tempDir->d_name).c_str());
				#pragma omp task untied
				deriveFreq(files,dirName+"/"+str);
			}
			//File is a regular one
			else
			{
				//Add file to vector
				int threadNo = omp_get_thread_num();
				files[threadNo].push_back(dirName+"/"+str);
			}
		} 
	}

	closedir(ds);
}

void processFile(unordered_map<string,int> docFreq[], string filename)
{
	int threadNo = omp_get_thread_num();

	// Set containing the words found in the file
	set<string> wordsInFile;

	ifstream inputFile;
	string readLine;
   	
	// Opening the file
	inputFile.open(filename);

 	while(getline(inputFile, readLine))
	{
	    if(readLine.empty())
	    {    
	        continue;
	    }

	    char* lineString = &readLine[0];
	    
	   	//Tokenize the line using punctuation marks
	    char* token;

	    while((token = strtok_r(lineString,",./;-!?@&(){}[]<>:'\" \r",&lineString)))
	    {
	    	for(int k=0;k<strlen(token);k++)
	        {
	            token[k] = tolower(token[k]);
	        }

	        string str(token); 
	        
	        if(wordsInFile.find(str) == wordsInFile.end())
	        {
	        	docFreq[threadNo][str]++;
	        	wordsInFile.insert(str);
	        }
	    }
	}
    
	//Closing the file
	inputFile.close();
}

unordered_map<string,int> reduceMaps(unordered_map<string,int> map1, unordered_map<string,int> map2)
{
	unordered_map<string,int> :: iterator iter;
	if(map1.size() > map2.size())
	{
		for(iter=map2.begin();iter!=map2.end();iter++)
		{
			map1[iter->first] += iter->second;
		}
		
		return map1;
	}
	else
	{
		for(iter=map1.begin();iter!=map1.end();iter++)
		{
			map2[iter->first] += iter->second;
		}
		
		return map2;
	}
}

int main (int argc, char *argv[]) 
{
	//Number of threads
	int nthreads = atoi(argv[1]);

	//Setting the number of threads
	omp_set_num_threads(nthreads);

	//Root directory name
	string root = "Hello/";
	
	//Unordered map to store the document freq for each word
	unordered_map<string,int> documentFreq[nthreads];
	vector<string> files[nthreads];
	vector<string> allFiles;

	double start_time = omp_get_wtime();

	/* Fork a team of threads giving them their own copies of variables */
	#pragma omp parallel
	{	
		int threadNo = omp_get_thread_num();
		
		#pragma omp single
		{
			deriveFreq(files,root);
		}			
		
		#pragma omp barrier

		// To ensure thread safety of the global vector
		#pragma omp critical
		{
			// Merge each thread's vector containing name of files into a global vector
			allFiles.insert(allFiles.end(),files[threadNo].begin(),files[threadNo].end());
		}

		#pragma omp barrier

		// Parallel File processing
		//////////////////////////////////////////////////////////////////////////////// dynamic to static and dynamic with chunk defined
		#pragma omp for schedule(dynamic)
		for(int i=0;i<allFiles.size();i++)
		{
			processFile(documentFreq,allFiles[i]);	
		}
		
		#pragma omp barrier

		// Parallel reduce	
		int level = nthreads;
		int index = 2;
		while(level!=1)
		{
			if(omp_get_thread_num()%index == 0)
			{
				documentFreq[omp_get_thread_num()] = reduceMaps(documentFreq[omp_get_thread_num()],documentFreq[omp_get_thread_num() + (index/2)]);
			}

			level /= 2;
			index *= 2;

			#pragma omp barrier
		}

	}  /* All threads join master thread and disband */

	double time = omp_get_wtime() - start_time;
	
	unordered_map<string,int> :: iterator iter;
	for(iter=documentFreq[0].begin();iter!=documentFreq[0].end();iter++)
	{
		cout << iter->first << ":"  << iter->second << endl;
	}

	printf("Time: %lf\n", time);

	return 0;

}
