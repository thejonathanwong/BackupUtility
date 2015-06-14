//#include <fstream>
#include <cstdlib>
//#include <cstdio>
//#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/sendfile.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
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
	while(inputString.back() == '/') {
			inputString.pop_back();
	}
	while(targetString.back() == '/') {
		targetString.pop_back();
	}

	string filename = inputString;
	auto pos = inputString.rfind("/") + 1;
	if( pos != string::npos ) {
		filename = inputString.substr(pos, string::npos);
	}

	//scans target directory and adds hashes of regular files to hashtable
	scanDirectory(tarFD, targetString);
	handleFD(inFD, inputString, targetString + "/" + filename);


//	struct stat statBuffer;
//	if(fstat(inFD, &statBuffer) == 0) {
//
//		if( statBuffer.st_mode & S_IFREG ) {
//			//input is a regular file
//
//			if( !handleFile(inFD) ) {
//				cerr << "Error copying " << inputString << " to " << targetString << "\n";
//			}
//
//		} else if ( statBuffer.st_mode & S_IFDIR ) {
//			//input is a directory
//			cout << "Input is a directory\n";
//			handleDir(inFD, inputString, targetString);
//
//		} else {
//			//input is symlink or other
//		}
//
//	} else {
//		cerr << "Error in fstat of input\n";
//	}

//	close(tarFD);
	close(inFD);

	for( auto it = filesFound.begin(); it != filesFound.end(); ++it ) {
		cout << it->first << " " << it->second << "\n";
	}
	cout << filesFound.size() << "\n";

}

bool backup::handleFD(int& input, string inputPath, string targetPath){
	bool success = false;
	

	
//	cout << inputName << " " << filename << " " << targetPath << "\n";

	struct stat inputStat;
	if( fstat(input, &inputStat) == 0 ) {

		if( inputStat.st_mode & S_IFREG ) {
			//input is regular file
			success = handleFile(input, targetPath);
		} else if ( inputStat.st_mode & S_IFDIR ) {
			//input is a directory
//			dirsFound.push(inputPath);
			success = handleDir(input, inputPath, targetPath);
		} else {
			//input is other
		}
		
	}

	return success;
}

bool backup::handleDir(int& input, string& inputPath, string& targetPath) {
//bool backup::handleDir(string inputPath, string targetPath) {
	bool success = true;

	//opens the input directory
	DIR * dir;
	if(NULL == (dir = fdopendir(input))) {
		cerr << "Cannot open directory " << inputPath;
		return false;
	}	

//	DIR * dir;
//	if(NULL == (dir = opendir(inputPath.c_str()))) {
//		cerr << "Cannot open directory " << inputPath;
//		return false;
//	}

	int status;
	if( (status = mkdir(targetPath.c_str(), 0644)) == 0  || errno == EEXIST ) {
		if(errno == EEXIST) {
			int targetFD;
			if( (targetFD = open(targetPath.c_str(), O_RDONLY, 0)) < 0 ) {
				cerr << "Unable to open " << targetPath << " to read\n";
			} else {
				scanDirectory(targetFD, targetPath);
			}
		}

		int currFD;
		struct dirent * curr;
		while( (curr = readdir(dir)) ) {
			string currName {curr->d_name};
			if(currName.compare(".") == 0) { continue; } 
			if(currName.compare("..") == 0) { continue; }

			string childPath { inputPath + "/" + currName };
			string newTarget { targetPath + "/" + currName };

			//opens file descriptor of the current file
//			if ( (currFD = openat(input, curr->d_name, O_RDONLY, 0)) < 0 ) {
			if ( (currFD = open(childPath.c_str(), O_RDONLY, 0)) < 0) {
				cerr << "Unable to open " << childPath << " to read\n";
				continue;
			}

			if( handleFD(currFD, childPath, newTarget) == false ) {
				success = false;
			}

			//close file descriptor of curr
			close(currFD);
		}

	}

	//closes input directory stream
	closedir(dir);

	return success;
}

//
//bool backup::handleDir(int& indir, string currDir, string targetDir ) {
//	bool output = false;
//
//	//opens the input directory
//	DIR * dir;
//	if(NULL == (dir = fdopendir(indir))) {
//		cerr << "Cannot open directory " << targetString;
//		return output;
//	}	
//
//	targetDir += "/" + currDir;
//
//	int status;
//	if( (status = mkdir(targetDir.c_str(), 0644)) == 0  || errno == EEXIST ) {
//		cout << "mkdir status " << status << " exists " << (errno == EEXIST) << "\n";
//
//		if(errno == EEXIST) {
//			int tarFD;
//			if( (tarFD = open(targetDir.c_str(), O_RDONLY, 0)) < 0 ) {
//				cerr << "Unable to open " << targetDir << " to read\n";
//			} else {
//				scanDirectory(tarFD, targetDir);
//			}
//		}
//
//		//loops through file in directory
//		struct stat currStat;
//		struct dirent * file;
//		int currFD;
//		while((file = readdir(dir))) {
//			string filename {file->d_name};
//			if(filename.compare(".") == 0) { continue; } 
//			if(filename.compare("..") == 0) { continue; }
//
//			//			string targetString {targetDir + "/" + filename};
//
//			//opens file descriptor of the current file
//			if ( (currFD = openat(indir, file->d_name, O_RDONLY, 0)) < 0 ) {
//				//			if ( (currFD = open(file->d_name, O_RDONLY, 0)) < 0) {
//				cerr << "Unable to open " << filename << " to read\n";
//				break;
//			}
//
//
//			//determines what is the current file descriptor
//			if(fstat(currFD, &currStat) == 0) {
//				if( (currStat.st_mode & S_IFREG) ){
//					//is a regular file
//					cout << targetDir + "/" + filename << "\n";
//					handleFile(currFD, targetDir + "/" + filename);
//				} else if ( (currStat.st_mode * S_IFDIR) ) {
//					//is a directory
//					cout << currFD << "\n";
//					cout << targetDir + "/" + filename << "\n";
//					handleDir(currFD, filename, targetDir);
//				}
//			} else { 
//				cerr << "Could not fstat " << filename << "\n";
//			}
//
//
//			close(currFD);
//			output = true;
//		}
//	} else {
//		cerr << "Error in creating directory " << targetString << "\n";
//		cout << "mkdir status " << status << " exists " << (errno == EEXIST) << "\n";
//	}
//
//	closedir(dir);
//	return output;
//}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handleFile
 *  Description:  Manages what to do with backing up a single file. If the file is not in the
 *  			  target directory, it is copied. If the file is already in the target directory, but the
 *  			  overwrite boolean is true, then the file is overwritten. Otherwise, nothing is done.
 * 
 * 				  @param: infile input file descriptor of the file to be backed up
 * 			
 * 				  @return: true if file was handled, false if error in copying
 * =====================================================================================
 */
//bool backup::handleFile(int& infile) {
//	//string of path to destination file
//	string destString = targetString + "/" + inputString;
//
//	return handleFile(infile, destString);
//}

// @param: destString path including name of the file to be created at the destination
//bool backup::handleFile(int& infile, string destString) {
bool backup::handleFile(int& input, string& targetPath) {
	bool success = false;

//	cerr << "inputName = " << inputName << " targetPath = " << targetPath << "\n";

	//if file is not in the hashset, then write the file to the output directory
	if(filesFound.find(targetPath) == filesFound.end()) {
		cout << "file not found\n";


		success = copyFile( input, targetPath );


	} else if(overwrite){
		cout << "file found\n";
		//file is found so check hash
		//hashes input file
		hasher.hash(input);

		//if hash of input file does not match the hash of the file in the target directory
		if(filesFound[targetPath] != hasher.toHex()) {
			cout << "hash does not match\n";
			success = copyFile( input, targetPath );
		}
	}	
	return success;
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
//bool backup::copyFile(int& infile, string& destString) {
//bool backup::copyFile(int& input, string& inputName, int& target) {
bool backup::copyFile(int& input, string& targetPath) {



	//resets error flag and moves pointer to start of the file
	lseek(input, 0, SEEK_SET);

	int flags = O_WRONLY | O_CREAT | O_TRUNC;
	//	if(overwrite) {
	//		flags |= O_TRUNC;
	//	}
	//opens and writes to destination file
	int dest;
//	if( (dest = openat(target, inputName.c_str(), flags, 0644)) > 0) {
	if( (dest = open(targetPath.c_str(), flags, 0644)) > 0) {

		char buffer[BUFFER];
		size_t size;
		while( (size = read(input, buffer, BUFFER)) > 0 ) {
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
		cerr << "Unable to copy file to " << targetPath << " " << "\n";
		perror("copyFile");
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
bool backup::scanDirectory(int tarFD, string tarString) {

	DIR * dir;
	if(NULL == (dir = fdopendir(tarFD))) {
		cerr << "Cannot open directory " << tarString;
		return false;
	}
	bool output = scanDirectory(dir, tarString);

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
 * 				  TODO: handle nested directories
 *
 * 				  @param: dirname is the name of the target directory to be scanned.
 *
 * 				  @return: true if the directory was opened, false otherwise.
 * =====================================================================================
 */
//bool backup::scanDirectory(char * dirname) {
bool backup::scanDirectory(DIR * dir, string& targetDir) {
	bool output = false;
	cout << "scanDirectory tarString = " << targetDir << "\n";

//	int dirFD;
//	if( (dirFD = dirfd(dir)) < 0) {
//		perror("scanDirectory: Cannot get file descriptor from DIR *");
//		return output;
//	}
//



	//	path dirPath (targetString);



	//loop to add hashes of the files in the output directory to the hashset
	struct stat currStat;
	struct dirent * file;
	int currFD;
	while((file = readdir(dir))) {
		string filename {file->d_name};

		//		if(strcmp(file->d_name, ".") == 0) 
		if(filename.compare(".") == 0) 
			continue;
		if(filename.compare("..") == 0) 
			continue;

		//path of the file found including the directory path
		//		string fullPath {dirPath + "/" + filename}
		//		path filePath = dirPath / filename;

		//if file is a regular file then add it to the hash table
		//		if( is_regular_file(filePath) ) {

		string fullPath {targetDir + "/" + filename};

		//checks if the file can be opened
//		if ( (currFD = openat(dirFD, file->d_name, O_RDONLY, 0)) < 0) {
		if ( (currFD = open(fullPath.c_str(), O_RDONLY, 0)) < 0) {
			cerr << "Unable to open " << fullPath << " to read\n";
			perror("scanDirectory");
			continue;
		}


		//determines what is the current file descriptor
		if(fstat(currFD, &currStat) == 0) {
			if( (currStat.st_mode & S_IFREG) ){
				//is a regular file

				//hashes the file and inserts into hashtable
				hasher.hash(currFD);
				filesFound[fullPath] = hasher.toHex();



				//					//checks if the file found can be opened
				//					ifstream fileFound { filePath.string(), ifstream::in | ifstream::binary };
				//					if(fileFound == NULL) {
				//						cerr << "File " << filename << " could not be opened.\n";
				//						//			fprintf(stderr, "File %s could not be opened\n", file->d_name);
				//					} else {
				//
				//						//hashes the file found and inserts into the hashet
				//						hasher.hash(fileFound);
				//						filesFound[file->d_name] = hasher.toHex();
				//
				//						//closes the file
				//						fileFound.close();
				//					}
			}
		} else { cerr << "Could not fstat " << filename << "\n"; }

		close(currFD);
		output = true;
	}

	return output;
}


int main(int argc, char * argv[]) {

	if(argc != 3) {
		//		cerr << "USAGE: " << argv[0] << " <Input File> <Output Directory> \n";
		fprintf(stderr, "USAGE: %s <Input File> <Output Directory>\n", argv[0]);
		return 1;
//		exit(1);
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
//		bool recCopyDir(path inpath, path target) {
//			path destpath = (target / inpath);
//
//			if( !exists(destpath) ) {
//				copy_directory( inpath, target );
//			} else if( !is_directory(destpath) ) {
//				cerr << destpath << " is not a directory\n";
//				return false;
//			}
//
//
//			for(auto it = directory_iterator(inpath); it != directory_iterator(); ++it) {
//
//
//			}
//
//			return true;
//
//		}

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
