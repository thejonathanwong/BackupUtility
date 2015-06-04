#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <unordered_set>
#include "md5.hpp"


int main(int argc, char * argv[]) {

	if(argc != 3) {
//		cerr << "USAGE: " << argv[0] << " <Input File> <Output Directory> \n";
		fprintf(stderr, "USAGE: %s <Input File> <Output Directory>\n", argv[0]);
		exit(1);
	}

	char * filename = argv[1];
	char * dirname = argv[2];

	DIR * dir;
	if(NULL == (dir = opendir(dirname))) {
		fprintf(stderr, "Cannot open directory %s\n", dirname);
		exit(1);
	}

	string dirString {dirname};

	md5 hasher;
	unordered_set<string> filesFound;

	struct dirent * file;
	while((file = readdir(dir))) {
		if(strcmp(file->d_name, ".") == 0) 
			continue;
		if(strcmp(file->d_name, "..") == 0) 
			continue;

		string path {dirString + "/" + file->d_name };

		ifstream infile { path, ifstream::in | ifstream::binary };
		if(infile == NULL) {
			//		cerr << "File " << filename << " could not be opened.\n";
			fprintf(stderr, "File %s could not be opened\n", file->d_name);
			continue;
		}


		hasher.hash(infile);
		cout << "Hash of " << file->d_name << ": " << hasher.toHex() << "\n";

		filesFound.insert(hasher.toHex());


		infile.close();
	}

	ifstream backupFile { filename, ifstream::in | ifstream::binary };
	if(backupFile == NULL) {
		fprintf(stderr, "File %s could not be opened\n", filename);
	} else {
		hasher.hash(backupFile);
		if(filesFound.count(hasher.toHex()) != 0) {
			cout << filename << " found\n";
		}

		backupFile.close();
	}



//	for(auto iter = filesFound.begin() ; iter != filesFound.end(); ++iter)
//		cout << *iter << "\n";


	closedir(dir);


	//	md5 m(infile);
	//	cout << m.toHex() << "\n";
	//	m.hash("");
	//	cout << m.toHex() << "\n";
	//	md5 mm("The quick brown fox jumps over the lazy dog");
	//	cout << "md5 of \"grape\": " << mm.toHex() << "\n";






	return 0;
}
