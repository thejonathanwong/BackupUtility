#include <fstream>
//#include <cstdio>
//#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <fcntl.h>
#include <unordered_set>
#include "md5.hpp"


int main(int argc, char * argv[]) {

	if(argc != 3) {
//		cerr << "USAGE: " << argv[0] << " <Input File> <Output Directory> \n";
		fprintf(stderr, "USAGE: %s <Input File> <Output Directory>\n", argv[0]);
		exit(1);
	}

	char * filename = argv[1];
//	string filename {argv[1]};
	char * dirname = argv[2];
	string dirString {dirname};

	//checks to see if output directory exists
	DIR * dir;
	if(NULL == (dir = opendir(dirname))) {
		fprintf(stderr, "Cannot open directory %s\n", dirname);
		exit(1);
	}


	md5 hasher;
	unordered_set<string> filesFound;

	//loop to add hashes of the files in the output directory to the hashset
	struct dirent * file;
	while((file = readdir(dir))) {
		if(strcmp(file->d_name, ".") == 0) 
			continue;
		if(strcmp(file->d_name, "..") == 0) 
			continue;

		//path of the file found including the directory path
		string path {dirString + "/" + file->d_name };

		//checks if the file found can be opened
		ifstream fileFound { path, ifstream::in | ifstream::binary };
		if(fileFound == NULL) {
			//		cerr << "File " << filename << " could not be opened.\n";
			fprintf(stderr, "File %s could not be opened\n", file->d_name);
			continue;
		}

		//hashes the file found and inserts into the hashet
		hasher.hash(fileFound);
		filesFound.insert(hasher.toHex());
//		cout << "Hash of " << file->d_name << ": " << hasher.toHex() << "\n";

		//closes the file
		fileFound.close();
	}

	//closes the output directory
	closedir(dir);





	//opens input file
	int fd;
	if((fd = open(filename, O_RDONLY, 0)) > 0) {
//		FILE * infile = fdopen(fd, "r");

		struct stat statBuffer;
		fstat(fd, &statBuffer);

//		hasher.hash(infile);
		hasher.hash(fd);
//		cout << "hash of " << filename << ": " << hasher.toHex() << "\n";

		//if file is not in the hashset, then write the file to the output director
		if(filesFound.count(hasher.toHex()) == 0) {
			
			//resets error flag and moves pointer to start of the file
//			clearerr(infile);
//			fseek(infile, 0, SEEK_SET);
			lseek(fd, 0, SEEK_SET);

			//creates destination file and writes to it
			string pathString = dirString + "/" + filename;
			const char * path = pathString.c_str();
			int dest;
			if( (dest = open(path, O_WRONLY | O_CREAT, 0644)) > 0) {

				//uses Linux kernel to copy the file so NOT PORTABLE TO OTHER OS
				if(sendfile(dest, fd, 0, statBuffer.st_size) != statBuffer.st_size) {
					fprintf(stderr, "Did not copy %s correctly\n", path);
				}

				close(dest);
			} else {
				fprintf(stderr, "Unable to open destination file %s\n", path);
			}


//			string path = dirString + "/" + filename;
//			ofstream dest { path };
//			if(dest == NULL) {
//				fprintf(stderr, "Cannot write %s to %s\n", filename, path.c_str());
//			} else {
//
//				//writes the file to the destination
//				dest << infile.rdbuf();
//
//				//closes thes stream
//				dest.close();
//			}
		}

		close(fd);
//		fclose(infile);
	} else {
		fprintf(stderr, "Unable to open input file\n");
	}
	





	return 0;
}

//void hashFile2(string filename) {
//	//opens the input file
//	ifstream infile { filename, ifstream::in | ifstream::binary };
//	if(infile == NULL || infile.fail()) {
//		fprintf(stderr, "File %s could not be opened\n", filename.c_str());
//	} else {
//
//		// hash the input file
//		hasher.hash(infile); 
//
//		//if file is not in the hashset, then write the file to the output director
//		if(filesFound.count(hasher.toHex()) == 0) {
//			
//			//resets error flag and moves pointer to start of the file
//			infile.clear();
//			infile.seekg(0);
//
//			//creates destination file and writes to it
//			string path = dirString + "/" + filename;
//			ofstream dest { path };
//			if(dest == NULL) {
//				fprintf(stderr, "Cannot write %s to %s\n", filename, path.c_str());
//			} else {
//
//				//writes the file to the destination
//				dest << infile.rdbuf();
//
//				//closes thes stream
//				dest.close();
//			}
//		}
//
//		//closes the input file
//		infile.close();
//	}
//}
