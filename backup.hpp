#ifndef __BACKUP_HPP__
#define __BACKUP_HPP__

#include <unordered_map>
#include "boost/filesystem.hpp"
#include "md5.hpp"
using namespace boost::filesystem;

class backup {

	public:



		void run(char * inName, char * tarName, bool ow);

	private:

		//members
		string inputString;
		string targetString;
		bool overwrite;
		md5 hasher;
		unordered_map<string, string> filesFound;

		bool scanDirectory(int tarFD);
		bool scanDirectory(DIR * dir);

		bool handleFile(int& infile);
		void copyFile(int& infile, string& destString);

		bool recCopyDir(path inDir, path target);
	
};

int main(int argc, char * argv[]);

#endif
