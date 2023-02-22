/*******************************************************************************
* This file is part of CLIah, Command Line Interface argument handler.
* If this file is included as part of a seperate project, please see the github
* page for this project: https://github.com/ADBeta/CLIah
*
* This project is under the GPL3.0 licence. (c) 2023 ADBeta
*******************************************************************************/

#include <iostream>
#include <string>
#include <vector>

#include <limits>

//NOTE: algorithm is used for strToUpper(), purely for practice with transform.
//If it is noted to be slow, or break some systems I will replace it with manual
//Index based looping on a string. Let me know.
#include <algorithm> 

#include "CLIah.hpp"

/*** Helper functions *********************************************************/
std::string strToUpper(std::string input) {
	//Transform seems to be the recommended method - causes another include 
	//and may cause other unknown issues. For practice, this will stay for now,
	//But may be replaced with a direct index (or iterator) in future.
	std::transform(input.begin(), input.end(), input.begin(), 
		//operation is getting the current char and performing toupper
		[](unsigned char chr){ return std::toupper(chr); }
	);
	
	//Return the uppercase string
	return input;
}

//TODO may cause errors compiling
void errorMsg(const unsigned int errLevel, const std::string errMsg) {
	std::string prefix;
	
	//Set prefix to Warning or Error depenging on errLevel
	if(errLevel == 0) { prefix = "Warning: ";
	} else { prefix = "Error: "; }
	
	std::cerr << prefix << errMsg << std::endl;
	
	//If the errLevel is > 1 then exit the program as a fatal error
	if(errLevel > 1) exit(EXIT_FAILURE);
}

/*** CLIah Functions **********************************************************/
namespace CLIah {
	namespace Config {
		//Are arbitrary strings enabled. If true, unknown args will be used as 
		//strings, if false, they will cause and error and exit
		bool stringsEnabled = false;
		//Verbosity selection. Prints matched Args. Defaults to false
		bool verbose = false;
		
	} //namespace Config

/*** Non API Functions ********************************************************/
//CLIah maxIndex vlaues is an alias of numeric limits unsigned int
unsigned int indexMax = std::numeric_limits<unsigned int>::max();

std::vector <Arg> argVector;
std::vector <String> stringVector;

void printArg(const Arg &ref) {
	//Print the argReference, primary and alias args.
	std::cout << "Name            | " << ref.argReference << std::endl;
	std::cout << "Argument(s)     | " << ref.priMatchStr << "  " 
	          << ref.aliasMatchStr << std::endl;
	
	//Print case sensitivity
	std::string caseStr;
	if(ref.caseSensitive == false) { caseStr = "False";
	} else { caseStr = "True"; }
	
	std::cout << "Case Sensitive  | " << caseStr << std::endl;
	
	//Print the type
	std::string typeStr;
	if(ref.type == ArgType::flag) typeStr = "Flag";
	if(ref.type == ArgType::subcommand) typeStr = "Subcommand";
	if(ref.type == ArgType::variable) typeStr = "Variable";
	
	std::cout << "Type            | " << typeStr << std::endl;
	
	//If the type is Subcommand or Variable, print the substring (if detected)
	if(ref.type == ArgType::subcommand || ref.type == ArgType::variable) {
		std::cout << "Substring       | " << ref.substring << std::endl;
	}
	
	//Print the index
	std::cout << "Index           | " << ref.index << std::endl;
	
	std::cout << std::endl;
}

void printString(const String &ref) {
	std::cout << "String          | " << ref.string << std::endl;
	std::cout << "Index           | " << ref.index << std::endl;
	
	std::cout << std::endl;
}

bool argStringsMatch(const Arg &ref, std::string input) {
	//Copy the Arg match strings into strings we can modify
	std::string priStr = ref.priMatchStr;
	std::string aliasStr = ref.aliasMatchStr;
	
	//Case sensitivity check. If the Arg isn't case sensitive then convert
	//Pri Alias and input to uppercase to make any case usage irrelevant
	if(ref.caseSensitive == false) {
		//Convert input, priStr and aliasStr to uppercase
		input = strToUpper(input);
		priStr = strToUpper(priStr);
		aliasStr = strToUpper(aliasStr);
	}
	
	//If an exact match for either pri or alias occurs, return a match flag
	if(priStr.compare(input) == 0 || aliasStr.compare(input) == 0) {
		return true;
	}
		
	//If no match is found return false
	return false;
}

/*** API Functions ************************************************************/
void analyseArgs(int argc, char *argv[]) {
	//Regular argc and argv should have been passed. Adjust to remove argv[0]
	--argc;
	++argv;
	
	//Bool flag when a match happens. Used to catch errors
	bool matchFound;

	//Go through every element in the argv array
	for(int argStrIdx = 0; argStrIdx < argc; argStrIdx++) {
		//Convert curent argv char[] to string
		std::string inputArg = argv[argStrIdx];
		
		//Loop through all Args stored in argVector. Use iterator
		std::vector<Arg>::iterator itrtr; 
		for(itrtr = argVector.begin(); itrtr != argVector.end(); itrtr++) {

			 
			//Set matched flag to the return bool of argStringsMatch and check it 
			if( (matchFound = argStringsMatch(*itrtr, inputArg)) == true) {
			
				/*** Arg Type detection ***************************************/
				//Get the matched argument type to do specific execution.
				CLIah::ArgType tempType = itrtr->type;
				
				//If the Arg is a Flag Type, no special executuion takes place
				//Flag type --
				
				//If the Arg is Subcommand type, get the next inputArg string,
				//set the substring of the main object
				if(tempType == CLIah::ArgType::subcommand) {
					//Make sure there *IS* a next argument, if not fatal error.
					if(argStrIdx + 1 >= argc) {
						errorMsg(2, "analyseArgs: " + inputArg +
						          " is a Subcommand but has no string");
					}
					
					//Incriment argStrIdx to get the substring, and the next
					//loop will go past it
					++argStrIdx;
					itrtr->substring = (std::string)argv[argStrIdx];
				}
				
				//TODO Variable type	
				
				/*** Constant execution when a match is found *****************/
				//Set the current argVector object detection flag to true
				itrtr->detected = true;
				//Set the index to argStrIndex
				itrtr->index = argStrIdx;
				
				//If verbosity is enabled, print the matched arg
				if(Config::verbose == true) {
					std::cout << "A match was found for \"" 
					          << inputArg << "\"" << std::endl;
					printArg(*itrtr);
				}
				
				//Break to prevent looping more Args looking for a match
				break;
			}//End of match found routine
			
		} //End of ArgVector loop
		
		/*** Error Handling ***************************************************/
		//If no match was found for inputArg after checking all argVector objs
		if(matchFound == false) {
			//If Strings are enabled
			if(Config::stringsEnabled == true) {
				//Set a temp String object to the values from argv[] and argc
				String tempString;
				tempString.string = inputArg;
				tempString.index = argStrIdx;
				stringVector.push_back(tempString);
				
				//If verbose is enabled, print the string
				if(Config::verbose == true) {
					std::cout << "A string was found" << std::endl;
					printString(tempString);
				}
			} else {
				std::string errStr = "No match for Arg \"" + inputArg + "\"";
				errorMsg(2, errStr);
			}
		} //End of error handler 
		
	} //End of inputArg loop
}

void addNewArg(const std::string ref, const std::string pri, const ArgType type,
               const std::string alias, const bool caseSensitive) {
	//Create an Arg object to set the parameters to
	Arg newArg;
	
	//Set the new args variables
	newArg.argReference = ref;
	newArg.priMatchStr = pri;
	newArg.type = type;
	newArg.aliasMatchStr = alias;
	newArg.caseSensitive = caseSensitive;
	
	//Push the new argument to the argVector
	argVector.push_back(newArg);
}

Arg getArgByReference(const std::string refStr) {
	static Arg retArg;
	
	//Go through every argVector element and check for reference string
	std::vector<Arg>::iterator itrtr; 
	for(itrtr = argVector.begin(); itrtr != argVector.end(); itrtr++) {
		//Compare reference string and argReference. 
		if(refStr.compare( itrtr->argReference ) == 0) {
			//Set the return Arg object pointer, and return it
			retArg = *itrtr;
			return retArg;
		}
	}
	
	//If no match is found, fatal error as this could cause segfaults
	errorMsg(2, "getArgByReference: No Arg exists using reference " + refStr);
	
	//Return to avoid compile warning
	return retArg;
}

Arg getArgByIndex(unsigned int index) {
	static Arg retArg;
	
	//Go through every argVector element and check for reference string
	std::vector<Arg>::iterator itrtr; 
	for(itrtr = argVector.begin(); itrtr != argVector.end(); itrtr++) {
		//Compare reference string and argReference. 
		if(index == itrtr->index) {
			//Set the return Arg object pointer, and return it
			retArg = *itrtr;
			return retArg;
		}
	}
	
	//If no match is found, fatal error as this could cause segfaults
	errorMsg(2, "getArgByIndex: No Arg exists at index " + index);
	
	//Return to avoid compile warning
	return retArg;
}

String getStringByIndex(unsigned int index) {
	static String retString;
	
	//Go through every argVector element and check for reference string
	std::vector<String>::iterator itrtr; 
	for(itrtr = stringVector.begin(); itrtr != stringVector.end(); itrtr++) {
		//Compare reference string and argReference. 
		if(index == itrtr->index) {
			//Set the return Arg object pointer, and return it
			retString = *itrtr;
			return retString;
		}
	}
	
	//If no match is found, fatal error as this could cause segfaults
	errorMsg(2, "getStringByIndex: No String exists at index " + index);
	
	//Return to avoid compile warning
	return retString;
}

bool isDetected(const std::string refStr) {
	//Get the Arg by reference and assign it to an Arg object
	Arg tempArg = getArgByReference(refStr);
	
	return tempArg.detected;
}

std::string getSubstring(const std::string refStr) {
	//Get the Arg by reference and assign it to an Arg object
	Arg tempArg = getArgByReference(refStr);
	
	return tempArg.substring;
}

} //namespace CLIah
