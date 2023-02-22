/*******************************************************************************
* This file is part of codelines. Please see the github:
* https://github.com/ADBeta/codelines
*
* codelines is a program to count the lines of -code- in a file or recursively
* in a directory, ignoring blank lines, spaces and comments. Built mainly for 
* C++ but has CLI flags to change the language.
*
* (c) ADBeta
* v0.4
* 22 Feb 2023
*******************************************************************************/
#include <iostream>
#include <string>

#include "TeFiEd.hpp"
#include "CLIah.hpp"


//Types of comments that can be detected and returned by isLineComment.
//Examples using C/C++ style:
//none
//single              //comment
//multi-single        /** comment **/
//multi-start         /* comment .....
//multi-end           ..... comment */
enum class CommentType { none, single, multiSingle, multiStart, multiEnd };

//Returns CommentType (see above) of the comment, if any, is found in the line.
CommentType lineCommentType(std::string fileLine) {
	//Get the position of the first non-empty char
	size_t firstCharPos = fileLine.find_first_not_of(" \t");

	//If the first chars are // the line is a single comment
	if(fileLine.substr(firstCharPos, 2) == "//") return CommentType::single;
	
	//Try to find the start and end multiline strings.
	size_t multiStartPos = fileLine.find("/*");
	size_t multiEndPos = fileLine.find("*/");
	
	//If there is a /* in the line
	if(multiStartPos != std::string::npos) {
		//If there is an end string
		if(multiEndPos != std::string::npos) {
			if(multiEndPos > multiStartPos) {
				//If the end string is found, and is after the start string, 
				//this line is a multi-single (eg. /* comment */)
				return CommentType::multiSingle;
			} else {
				//If end appears before start, this is two multiline strings 
				//butted against eachother (eg. ...comment 1 */ /* comment 2...)
				return CommentType::multiStart;
			}
		}
			
		//If there is no end string, this is just a multiline start, this can
		//cause issues, as "code; /* comment..." is a valid line, but will be
		//returned as invalid. Keep and eye on this issue
		return CommentType::multiStart;
	}

	//Check if there is an end string now that start and combo strings are done
	if(multiEndPos != std::string::npos) {
		//again, there could be an issue here with the end of a comment, but
		//some valid code after it. keep an eye for this (eg. comment */ code;)
		return CommentType::multiEnd;
	}

	//If no conditions are met, this line is not a comment
	return CommentType::none;
}

//Returns true/false if the line passed is a valid code line. Adjusts rules
//based on which language is selected.
bool isValidLine(std::string fileLine) {
	//Get the position of the first non-empty char
	size_t firstCharPos = fileLine.find_first_not_of(" \t");

	//If multiline start has been seen but the end has not. Guards a multiline
	static bool multilineComment = false;

	//Regardless of language, an empty line or only tabs or spaces is not valid
	if(fileLine.empty() || firstCharPos == std::string::npos) return false;
	
	//Get the line comment type for later
	CommentType lType = lineCommentType(fileLine);
	
	//Simple checks
	if(lType == CommentType::single || lType == CommentType::multiSingle) {
		return false;
	}
	
	//Setup the multiline flag
	if(lType == CommentType::multiStart) {
		multilineComment = true;
		return false;
	}
	
	if(lType == CommentType::multiEnd) {
		multilineComment = false;
		return false;
	}
	
	//If multiline is active currently, just fail
	if(multilineComment == true) return false;
	
	//If the line hasn't failed yet, it is a valid line
	return true;
}

/*** Main *********************************************************************/
int main(int argc, char *argv[]){
	
	
	//TeFiEd input file pointer - gets swapped multiple times.
	TeFiEd *codeFile;;
	
	
	//Create a new TeFiEd object with text file
	codeFile = new TeFiEd("./test.txt");
	
	codeFile->setVerbose(true);
	
	//Read in the file, with error handling
	codeFile->read();
	
	for(size_t x = 1; x < codeFile->lines() + 1; x++) {
		std::string line = codeFile->getLine(x);
		std::cout << line << "\t\t" << isValidLine(line) << std::endl;
	}
	
	//Clear RAM of the TeFiEd object ready for end or next loop.
	delete codeFile;
	
	return 0;
}
