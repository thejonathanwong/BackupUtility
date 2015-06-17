#ifndef __BACKUP_HPP__
#define __BACKUP_HPP__

#include <unordered_map>
#include <queue>
#include <utility>
//#include "boost/filesystem.hpp"
#include "md5.hpp"
//using namespace boost::filesystem;

class backup {

	public:



		void run(char * inName, char * tarName, bool ow);
		void run2(char * inName, char * tarName, bool ow);

	private:

		typedef pair<string, string> pathPair;

		//members
		string inputString;
		string targetString;

		bool overwrite;
		md5 hasher;

		unordered_map<string, string> filesFound;
		queue< pathPair > pathQueue;

		bool scanDirectory(int tarFD, string tarString);
		bool scanDirectory(DIR * dir, string& tarString);

		bool handleDir(int& input, string& inputPath, string& targetPath);
//		bool handleDir(string inputPath, string targetPath);

//		bool handleFile(int& infile);
//		bool handleFile(int& infile, string destString);

		bool handleFile(int& input, string& targetPath);
//		bool copyFile(int& infile, string& inputName, int& target);
		bool copyFile(int& infile, string& targetPath);

		bool handleFD(int& input, string inputName, string targetPath);
//		bool recCopyDir(path inDir, path target);
	
};

int main(int argc, char * argv[]);

#endif
