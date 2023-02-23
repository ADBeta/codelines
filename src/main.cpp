/*******************************************************************************
* This file is part of codelines. Please see the github:
* https://github.com/ADBeta/codelines
*
* codelines is a program to count the lines of -code- in a file or recursively
* in a directory, ignoring blank lines, spaces and comments. Built mainly for 
* C++ but has CLI flags to change the language.
*
* (c) ADBeta
* v0.6
* 23 Feb 2023
*******************************************************************************/
#include <iostream>
#include <string>

#include "TeFiEd.hpp"
#include "CLIah.hpp"

struct Language {
	//Comment strings
	std::string singleLineComment;
	std::string multiLineCommentBeg;
	std::string multiLineCommentEnd;

}; //struct Language

Language *selLang;


//Removed all the comment strings from the TeFiEd Vector
void removeSingleComments(TeFiEd &inFile) {
	//Go through all lines in the file
	for(size_t cLine = 1; cLine <= inFile.lines(); cLine++) {
		std::string lineStr = inFile.getLine(cLine);
		
		//Find any language specific single comment string
		size_t commentPos = lineStr.find(selLang->singleLineComment);
		if(commentPos != std::string::npos) {
			//If commentPos is a valid position, erase from that point to
			//the end of the string (Will leave blank lines behind)
			lineStr.erase(commentPos, std::string::npos);
			
			//Then remove the line we just modified
			inFile.removeLine(cLine);			
			//And push the new line to that index
			inFile.insertString(lineStr, cLine);
		}
	}
}

//Removed all the blank lines from the TeFiEd Vector
void removeBlankLines(TeFiEd &inFile) {
	//Current line
	size_t cLine = 1;
	
	while(cLine <= inFile.lines()) {
		std::string lineStr = inFile.getLine(cLine);
	
		//If the current line only has spaces or tabs, it is empty.
		if(lineStr.find_first_not_of(" \t") == std::string::npos) {
			//Delete that line from the file
			inFile.removeLine(cLine);			
		} else {
			//Otherwise the line has info, skip this line.
			//NOTE: Do not inc after remove because this could skip a possible
			//empty line
			++cLine;
		}
	}
}

/*** Main *********************************************************************/
int main(int argc, char *argv[]){
	/*** Command Line setup ***************************************************/
	//Configure CLIah
	CLIah::Config::verbose = true; //Set verbosity when match is found
	CLIah::Config::stringsEnabled = true; //Enable arbitrary strings
	
	//Language selection argument
	CLIah::addNewArg(
		"Language",                          //Reference
		"--language",                        //Primary match string
		CLIah::ArgType::subcommand,          //Argument type
		"-L"                                 //Alias match string
	);
	
	
	
	CLIah::analyseArgs(argc, argv);
	
	Language C_family;
	C_family.singleLineComment = "//";
	C_family.multiLineCommentBeg = "/*";
	C_family.multiLineCommentEnd = "*/";
	
	
	
	if(CLIah::isDetected("Language") == false) {
		//Default behaviour if language is not specified by the user
		selLang = &C_family;
	} else {
	
	}
	
	/**********************************/
	
	//TeFiEd input file pointer - gets swapped multiple times.
	TeFiEd *codeFile;;
	
	
	
	/**********************************/
	//Create a new TeFiEd object with text file
	codeFile = new TeFiEd("./test.txt");
	
	codeFile->setVerbose(true);
	
	//Read in the file, with error handling
	if(codeFile->read() != 0) {
		std::cerr << "Error: Unable to open file " << codeFile->filename() 
		          << ". Exiting" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	
	/*****/
	
	removeSingleComments(*codeFile);
	
	//Lastly remove the blank lines, which is also a cleanup from the last steps
	removeBlankLines(*codeFile);
	
	for(size_t cLine = 1; cLine <= codeFile->lines(); cLine++) {
		std::cout << codeFile->getLine(cLine) << std::endl;
	}
	
	//Clear RAM of the TeFiEd object ready for end or next loop.
	delete codeFile;
	
	return 0;
}
