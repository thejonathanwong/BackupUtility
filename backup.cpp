#include <fstream>
//#include <cstdio>
//#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <fcntl.h>
//#include <unordered_set>
//#include <unordered_map>
#include "backup.hpp"
//#include "md5.hpp"


//void backup(char * input, char * output);
void backup::run(char * inName, char * tarName, bool ow) {
	overwrite = ow;
	inputString = string{inName};
	targetString = string{tarName};

//	path inPath (inName);
//	path target (tarName);

	int inFD;
	if ( (inFD = open(inName, O_RDONLY, 0)) < 0) {
		cerr << "Unable to open input file " << inputString << "\n";
		return;
	}

	int tarFD;
	if ( (tarFD = open(tarName, O_RDONLY, 0)) < 0) {
		cerr << "Unable to open target directory " << targetString << "\n";
		return;
	}

	struct stat tarStat;
	if(fstat(tarFD, &tarStat) == 0) {
		if( !(tarStat.st_mode & S_IFDIR) ){
			// target is not a directory
			cerr << targetString << " is not a directory\n";
			return;
		}
	}
	

//	if( !exists(inPath) ) {
//		cerr << "Input file " << inPath << " does not exist\n";
//		return;
//	} else if ( (inFD = open(inName, O_RDONLY,0)) < 0) {
//		cerr << "Unable to open input file " << inPath << "\n";
//		return;
//	}
//
//	if( !is_directory(target) ) {
//		cerr << target << " is not a directory\n";
//		return;
//	} 

	//scans target directory and adds hashes of regular files to hashtable
	scanDirectory(tarFD);


	struct stat statBuffer;
	if(fstat(inFD, &statBuffer) == 0) {

		if( statBuffer.st_mode & S_IFREG ) {
			//input is a regular file

			if( !handleFile(inFD) ) {
				cerr << "Error copying " << inputString << " to " << targetString << "\n";
			}

		} else if ( statBuffer.st_mode & S_IFDIR ) {
			//input is a directory

		} else {
			//input is symlink or other
		}

	} else {
		cerr << "Error in fstat of input\n";
	}

	close(inFD);

}

bool backup::handleDir(int& indir, string& currDir, string& targetDir ) {
	DIR * dir;
	if(NULL == (dir = fdopendir(indir))) {
		cerr << "Cannot open directory " << targetString;
		return false;
	}	

	//loops through file in directory
	struct stat currStat;
	struct dirent * file;
	int currFD;
	vector<char *> 
	while((file = readdir(dir))) {
		string filename {file->d_name};
		if(filename.compare(".") == 0) 
			continue;
		if(filename.compare("..") == 0) 
			continue;

		//opens file descriptor of the current file
		if ( (currFD = open(file->d_name, O_RDONLY, 0)) < 0) {
			cerr << "Unable to open " << filename << " to read\n";
			break;
		}

		
		//determines what is the current file descriptor
		if(fstat(currFD, &currStat) == 0) {
			if( (currStat.st_mode & S_IFREG) ){
				//is a regular file
				handleFile(currFD, targetDir + "/" + filename);
			} else if ( (currStat.st_mode * S_IFDIR) ) {
				//is a directory
				handleDir(currFD, currDir + "/" + filename, targetDir + "/" + filename);
			}
		} else { 
			cerr << "Could not fstat " << filename << "\n";
		}


		close(currFD);
	}

	closedir(dir);
	return true;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handleFile
 *  Description:  Manages what to do with backing up a single file.
 * 
 * 				  @param: infile input file descriptor of the file to be backed up
 * 			
 * 				  @return: true if file was handled, false if error in copying
 * =====================================================================================
 */
bool backup::handleFile(int& infile) {
	//string of path to destination file
	string destString = targetString + "/" + inputString;

	return handleFile(infile, destString);
}

bool backup::handleFile(int& infile, string& destString) {
	
	//if file is not in the hashset, then write the file to the output directory
	if(filesFound.find(inputString) == filesFound.end()) {
		cout << "file not found\n";




		return copyFile(infile, destString);


	} else if(overwrite){
		cout << "file found\n";
		//file is found so check hash
		//hashes input file
		hasher.hash(infile);

		//if hash of input file does not match the hash of the file in the target directory
		if(filesFound[inputString] != hasher.toHex()) {
			cout << "hash does not match\n";
			return copyFile(infile, destString);
		}
	}	
	return true;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  copyFile
 *  Description:  Copies contents of a file descriptor to a target destination directory. Assumes
 *  			  input is a file, and not a directory/symlink/other.
 * 				  
 * 				  TODO: error handling on the write
 * 
 * 				  @param: infile file descriptor of the input
 * 				  @param: destString destination of where the file is to be copied to
 * 				  
 * 				  @return: true if destination was opened, false otherwise
 * =====================================================================================
 */
bool backup::copyFile(int& infile, string& destString) {
	//resets error flag and moves pointer to start of the file
	lseek(infile, 0, SEEK_SET);

	int flags = O_WRONLY | O_CREAT | O_TRUNC;
//	if(overwrite) {
//		flags |= O_TRUNC;
//	}
	//opens and writes to destination file
	int dest;
	if( (dest = open(destString.c_str(), flags, 0644)) > 0) {

		char buffer[BUFFER];
		size_t size;
		while( (size = read(infile, buffer, BUFFER)) > 0 ) {
			write(dest, buffer, size);
		}

		//
		//			//uses Linux kernel to copy the file so NOT PORTABLE TO OTHER OS
		//			//TODO: possibly switch to boost filesystem copy_file, but potentially slower but portable?
		////			if(sendfile(dest, fd, 0, statBuffer.st_size) != statBuffer.st_size) {
		////				fprintf(stderr, "Did not copy %s correctly\n", pathC);
		////			}
		//
		close(dest);
	} else {
		cerr << "Unable to open destination file " << destString << "\n";
		return false;
	}
	return true;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  scanDirectory
 *  Description:  Helper function to scanDirectory. Opens DIR * from file descriptor.
 *
 * 				  @param: tarFD file descriptor of input to be converted to DIR *
 * 		
 * 				  @return: true if directory was opened, false otherwise
 * =====================================================================================
 */
bool backup::scanDirectory(int tarFD) {
	
	DIR * dir;
	if(NULL == (dir = fdopendir(tarFD))) {
		cerr << "Cannot open directory " << targetString;
		return false;
	}
	bool output = scanDirectory(dir);

	//closes the output directory
	closedir(dir);
	
	return output;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  scanDirectory
 *  Description:  Scans the target directory and adds hashes of all top level regular files to
 *  			  filesFound hashtable.
 *	
 * 				  @param: dirname is the name of the target directory to be scanned.
 *
 * 				  @return: true if the directory was opened, false otherwise.
 * =====================================================================================
 */
//bool backup::scanDirectory(char * dirname) {
bool backup::scanDirectory(DIR * dir) {


	path dirPath (targetString);
	


	//loop to add hashes of the files in the output directory to the hashset
	struct dirent * file;
	while((file = readdir(dir))) {
		string filename {file->d_name};

//		if(strcmp(file->d_name, ".") == 0) 
		if(filename.compare(".") == 0) 
			continue;
		if(filename.compare("..") == 0) 
//		if(strcmp(file->d_name, "..") == 0) 
			continue;

		//path of the file found including the directory path
//		string fullPath {dirPath + "/" + filename}
		path filePath = dirPath / filename;

		//if file is a regular file then add it to the hash table
		if( is_regular_file(filePath) ) {

			//checks if the file found can be opened
			ifstream fileFound { filePath.string(), ifstream::in | ifstream::binary };
			if(fileFound == NULL) {
				cerr << "File " << filename << " could not be opened.\n";
				//			fprintf(stderr, "File %s could not be opened\n", file->d_name);
				continue;
			}

			//hashes the file found and inserts into the hashet
			hasher.hash(fileFound);
			filesFound[file->d_name] = hasher.toHex();

			//closes the file
			fileFound.close();
		}
	}



	
	return true;
}


int main(int argc, char * argv[]) {

	if(argc != 3) {
		//		cerr << "USAGE: " << argv[0] << " <Input File> <Output Directory> \n";
		fprintf(stderr, "USAGE: %s <Input File> <Output Directory>\n", argv[0]);
		exit(1);
	}

	char * filename = argv[1];
	char * dirname = argv[2];

	bool overwrite = true;
	backup b;
	b.run(filename, dirname, overwrite);
	

//	//checks input file and opens
//	int fd;
//	if((fd = open(filename, O_RDONLY,0)) < 0) {
//		fprintf(stderr, "Unable to open input file\n");
//		exit(1);
//	}
//
//	//checks to see if output directory exists
//	DIR * dir;
//	if(NULL == (dir = opendir(dirname))) {
//		fprintf(stderr, "Cannot open directory \"%s\"\n", dirname);
//		exit(1);
//	}
//
//
//	string dirString {dirname};
//	md5 hasher;
////	unordered_set<string> filesFound;
//	unordered_map<string, string> filesFound;
//
//	//loop to add hashes of the files in the output directory to the hashset
//	struct dirent * file;
//	while((file = readdir(dir))) {
//		if(strcmp(file->d_name, ".") == 0) 
//			continue;
//		if(strcmp(file->d_name, "..") == 0) 
//			continue;
//
//		//path of the file found including the directory path
//		string pathString {dirString + "/" + file->d_name };
//
//		//checks if the file found can be opened
//		ifstream fileFound { pathString, ifstream::in | ifstream::binary };
//		if(fileFound == NULL) {
//			//		cerr << "File " << filename << " could not be opened.\n";
//			fprintf(stderr, "File %s could not be opened\n", file->d_name);
//			continue;
//		}
//
//		//hashes the file found and inserts into the hashet
//		hasher.hash(fileFound);
//		filesFound[file->d_name] = hasher.toHex();
////		filesFound.insert(hasher.toHex());
//		//		cout << "Hash of " << file->d_name << ": " << hasher.toHex() << "\n";
//
//		//closes the file
//		fileFound.close();
//	}
//
//	//closes the output directory
//	closedir(dir);
//
//
//
//
//
//
//
//	struct stat statBuffer;
//	if(fstat(fd, &statBuffer) == 0) {
//
//		if( statBuffer.st_mode & S_IFDIR ) {
//			//input is a directory
//			cout << "Directory\n";
//
//			path dirpath (dirname);
//			path inpath (filename);
//
//
//			//input name and target directory share a name
//			if(inpath.compare(dirpath.filename()) == 0) {
//				cout << "Same dir name\n";
//			} else {
//				//input and target directory do not share a name
//				//copy directory
//				path destpath = (dirpath / inpath);
//			}
//
//			
//
//
//		} else if ( statBuffer.st_mode & S_IFREG ) {
//			//input is a file
//			cout << "File\n";
//
//
//
//
//			//if file is not in the hashset, then write the file to the output directory
////			if(filesFound.count(hasher.toHex()) == 0) {
//			if(filesFound.find(filename) == filesFound.end()) {
//				cout << "file not found\n";
//
//				//resets error flag and moves pointer to start of the file
//				lseek(fd, 0, SEEK_SET);
//
//				//string of path to destination file
//				string pathString = dirString + "/" + filename;
//				const char * pathC = pathString.c_str();
//
//				//opens and writes to destination file
//				int dest;
//				if( (dest = open(pathC, O_WRONLY | O_CREAT, 0644)) > 0) {
//
//					//uses Linux kernel to copy the file so NOT PORTABLE TO OTHER OS
//					//TODO: possibly switch to boost filesystem copy_file, but potentially slower but portable?
//					if(sendfile(dest, fd, 0, statBuffer.st_size) != statBuffer.st_size) {
//						fprintf(stderr, "Did not copy %s correctly\n", pathC);
//					}
//
//					close(dest);
//				} else {
//					fprintf(stderr, "Unable to open destination file %s\n", pathC);
//				}
//
//			} else {
//				cout << "file found\n";
//				//file is found so check hash
//				//hashes input file
//				hasher.hash(fd);
//
//				//if hash of input file does not match the hash of the file in the target directory
//				if(filesFound[filename] != hasher.toHex()) {
//					cout << "hash does not match\n";
//					
//				}
//			}
//
//
//
//		} else {
//			cout << "Not file or directory\n";
//		}
//	}
//
//
//
//
//
//	close(fd); //closes input file


	return 0;
}

//uses boost::filesystem to recursively copy an entire directory
bool recCopyDir(path inpath, path target) {
	path destpath = (target / inpath);

	if( !exists(destpath) ) {
		copy_directory( inpath, target );
	} else if( !is_directory(destpath) ) {
		cerr << destpath << " is not a directory\n";
		return false;
	}


	for(auto it = directory_iterator(inpath); it != directory_iterator(); ++it) {
		

	}

	return true;

}

//bool copyFile(path inpath, path target, md5 hasher) {
//	path destpath = (target / inpath);
//
//	if ( exists(destpath) && is_regular_file(destpath) ) {
//		
//	}
//	
//}

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
