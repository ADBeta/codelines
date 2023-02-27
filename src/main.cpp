/*******************************************************************************
* This file is part of codelines. Please see the github:
* https://github.com/ADBeta/codelines
*
* codelines is a program to count the lines of -code- in a file or recursively
* in a directory, ignoring blank lines, spaces and comments. Built mainly for 
* C++ but has CLI flags to change the language.
*
* (c) ADBeta
* v1.0
* 27 Feb 2023
*******************************************************************************/
#include <iostream>
#include <string>

#include "TeFiEd.hpp"
#include "CLIah.hpp"

struct Language {
	//Comment strings
	std::string singleComment;
	std::string multiCommentBeg;
	std::string multiCommentEnd;

}; //struct Language

Language *langSelected;


//Removed all the comment strings from the TeFiEd Vector
void removeSingleComments(TeFiEd &inFile) {
	//Go through all lines in the file
	for(size_t cLine = 1; cLine <= inFile.lines(); cLine++) {
		std::string lineStr = inFile.getLine(cLine);
		
		//Find any language specific single comment string
		size_t commentPos = lineStr.find(langSelected->singleComment);
		if(commentPos != std::string::npos) {
			//If commentPos is a valid position, erase from that point to
			//the end of the string (Will leave blank lines behind)
			lineStr.erase(commentPos, std::string::npos);
			
			//Replace the current line with the generated string
			inFile.replace(cLine, lineStr);
		}
	}
}

//Remove all the multiline comments in the file TODO
void removeMultiComments(TeFiEd &inFile) {
	const size_t npos = std::string::npos;
	
	static bool multiComment = false;
	
	//Go through all lines in the file
	size_t cLine = 1;
	while(cLine <= inFile.lines()) {
		std::string lineStr = inFile.getLine(cLine);
		
		//Get position of multi-end and multi-start strings
		size_t multiBegPos = lineStr.find(langSelected->multiCommentBeg);
		size_t multiEndPos = lineStr.find(langSelected->multiCommentEnd);
		//Nudge end pos along by the length of its string
		if(multiEndPos != npos) {
			multiEndPos = multiEndPos + langSelected->multiCommentEnd.size();
		}
		////////////////////////////////////////////////////////////////////////
		
		//If this line is not the start or end of a multi-comment string		
		if(multiBegPos == std::string::npos && multiEndPos == std::string::npos) {
			//If the current line is between multi-line comments, delete it
			if(multiComment == true) {
				inFile.remove(cLine);
			} else {
				++cLine;
			}
			
			//This line is done
			continue;
		}
				
		//If the multi-start string is detected 
		if(multiBegPos != std::string::npos) {
			//Remove from begin pos to end pos - If single line comment, just
			//removes the comment, if full line, will remove line to the end
			//due to std::string::npos no-match condition of find
			lineStr.erase(multiBegPos, multiEndPos);
			
			//Replace that line in the file
			inFile.replace(cLine, lineStr);
			
			//If there is no multi-end string, this is a true multi-line comment
			//This guards against single line comments using multi-line style
			if(multiEndPos == std::string::npos) {
				multiComment = true;
			}
			
			//Go to next line since this one is modified now.
			++cLine;
			
			continue;
		}
		
		//If the multi-end string is present on this line
		if(multiEndPos != std::string::npos) {
			//Multi comment is done
			multiComment = false;
			
			//Remove from the beginning of this line, to the end string
			lineStr.erase(0, multiEndPos);
			//Replace that line in the file
			inFile.replace(cLine, lineStr);
			
			//Next line
			++cLine;
			
			continue;
		}
		//Done
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
			inFile.remove(cLine);			
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
	
	//Help flag
	CLIah::addNewArg(
		"Help",                        //Reference
		"--help",                      //Primary match string
		CLIah::ArgType::flag,          //Argument type
		"-h"                           //Alias match string
	);
	
	//Output file subcommand
	CLIah::addNewArg(
		"Output",
		"--output",
		CLIah::ArgType::flag,
		"-o"
	);
	
	//Directory selector (?)
	
	//Language selection argument
	CLIah::addNewArg(
		"Language",
		"--language",
		CLIah::ArgType::subcommand,
		"-L"
	);
	
	//Get CLIah to read the arguments and decode what has been selected.
	CLIah::analyseArgs(argc, argv);
		
	/*** Argument handling ****************************************************/
	Language C_family;
	C_family.singleComment = "//";
	C_family.multiCommentBeg = "/*";
	C_family.multiCommentEnd = "*/";
	
	if(CLIah::isDetected("Language") == false) {
		//Default behaviour if language is not specified by the user
		langSelected = &C_family;
	} else {
	
	}
	
	//Check if there are any strings (files), if not, error and exit.
	if(CLIah::stringVector.size() == 0) {
		std::cerr << "no files. short help. exit" << std::endl;
		return 1;
	}
	
	/*** File Operations ******************************************************/
	//TeFiEd input file pointer - might get swapped multiple times.
	TeFiEd *codeFile;;
	
	//Keep track of the lines in the file
	size_t fileLines, codeLines;
	
	//Value to keep converted
	int codePercent;
	
	//Go through every string in the array, and try opening it as a file
	for(size_t cFile = 0; cFile < CLIah::stringVector.size(); cFile++) { 
		//Create a new TeFiEd object with text file from string vector
		codeFile = new TeFiEd(CLIah::stringVector.at(cFile).string);		
		
		//Read the passed file, error if cannot open the file.
		if(codeFile->read() != 0) {
			return 1;
		}
		
		//Print out the lines in the file before modification.
		fileLines = codeFile->lines();
		std::cout << codeFile->filename() << " Contains " << fileLines <<
		          " lines\t\t" << std::flush;
		
		//First remove the multi line comments
		removeMultiComments(*codeFile);
		//Remove the single comment lines from the file. leaves blank lines.
		removeSingleComments(*codeFile);
		//Lastly remove the blank lines, which is also a cleanup from the last steps
		removeBlankLines(*codeFile);
		
		//Print out the file (maybe add vebose?) TODO
		//for(size_t cLine = 1; cLine <= codeFile->lines(); cLine++) {
		//	std::cout << codeFile->getLine(cLine) << std::endl;
		//}
		
		//Print lines after modification, and calculate the percentage
		codeLines = codeFile->lines();
		codePercent = ((float)codeLines / (float)fileLines) * 100.00;
		std::cout << codeLines << " of which are code.\t\t" << "Which is "
		          << codePercent << "%" << std::endl;
		
		//Clear RAM of the TeFiEd object ready for end or next loop.
		delete codeFile;
	}

	//Execution finished.	
	return 0;
}
