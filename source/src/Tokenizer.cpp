
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Tokenizer.cpp
 * Description: My trusty old string tokenizer class.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "Tokenizer.h"
#include <wx/log.h>


/*******************************************************************
 * TOKENIZER CLASS FUNCTIONS
 *******************************************************************/

/* Tokenizer::Tokenizer
 * Tokenizer class constructor
 *******************************************************************/
Tokenizer::Tokenizer(bool c_comments, bool h_comments, bool s_comments) {
	// Initialize variables
	current = NULL;
	start = NULL;
	size = 0;
	comments = 0 | c_comments | h_comments << 1 | s_comments << 2;
	debug = false;
	special = ";,:|={}/";	// Default special characters
	name = "nothing";
	line = 1;
	t_start = 0;
	t_end = 0;
}

/* Tokenizer::~Tokenizer
 * Tokenizer class destructor
 *******************************************************************/
Tokenizer::~Tokenizer() {
	// Free memory if used
	if (start) free(start);
}

/* Tokenizer::openFile
 * Reads a portion of a file to the Tokenizer
 *******************************************************************/
bool Tokenizer::openFile(string filename, uint32_t offset, uint32_t length) {
	// Open the file
	wxFile file(filename);

	name = filename;

	// Check file opened
	if (!file.IsOpened()) {
		wxLogMessage("Tokenizer::openFile: Unable to open file %s", filename.c_str());
		return false;
	}

	// If length isn't specified or exceeds the file length,
	// only read to the end of the file
	if (offset + length > file.Length() || length == 0)
		length = file.Length() - offset;

	// Setup variables & allocate memory
	size = length;
	position = 0;
	start = current = (char *) malloc(size);
	line = 1;
	t_start = 0;
	t_end = 0;

	// Read the file portion
	file.Seek(offset, wxFromStart);
	file.Read(start, size);

	return true;
}

/* Tokenizer::openString
 * Reads a portion of a string to the Tokenizer
 *******************************************************************/
bool Tokenizer::openString(string text, uint32_t offset, uint32_t length, string source) {
	// If length isn't specified or exceeds the string's length,
	// only copy to the end of the string
	if (offset + length > (uint32_t) text.length() || length == 0)
		length = (uint32_t) text.length() - offset;

	// Setup variables & allocate memory
	size = length;
	position = 0;
	line = 1;
	t_start = 0;
	t_end = 0;
	start = current = (char *) malloc(size);
	name = source;

	// Copy the string portion
	memcpy(start, ((char*) text.char_str()) + offset, size);

	return true;
}

/* Tokenizer::openMem
 * Reads a chunk of memory to the Tokenizer
 *******************************************************************/
bool Tokenizer::openMem(const char* mem, uint32_t length, string source) {
	// Length must be specified
	if (length == 0) {
		wxLogMessage("Tokenizer::openMem: length not specified");
		return false;
	}

	name = source;

	// Setup variables & allocate memory
	size = length;
	position = 0;
	line = 1;
	t_start = 0;
	t_end = 0;
	start = current = (char*) malloc(size);

	// Copy the data
	memcpy(start, mem, size);

	return true;
}

/* Tokenizer::openMem
 * Reads a chunk of memory to the Tokenizer
 *******************************************************************/
bool Tokenizer::openMem(const uint8_t* mem, uint32_t length, string source) {
	// Length must be specified
	if (length == 0) {
		wxLogMessage("Tokenizer::openMem: length not specified");
		return false;
	}

	name = source;

	// Setup variables & allocate memory
	size = length;
	position = 0;
	line = 1;
	t_start = 0;
	t_end = 0;
	start = current = (char*) malloc(size);

	// Copy the data
	memcpy(start, mem, size);

	return true;
}

/* Tokenizer::openMem
 * Reads a chunk of memory to the Tokenizer
 *******************************************************************/
bool Tokenizer::openMem(MemChunk * mem, string source) {
	// Needs to be valid
	if (mem == NULL) {
		wxLogMessage("Tokenizer::openMem: invalid MemChunk");
		return false;
	}

	name = source;

	// Setup variables & allocate memory
	size = mem->getSize();
	position = 0;
	line = 1;
	t_start = 0;
	t_end = 0;
	start = current = (char*) malloc(size);

	// Copy the data
	memcpy(start, mem->getData(), size);

	return true;
}

/* Tokenizer::isWhitespace
 * Checks if a character is 'whitespace'
 *******************************************************************/
bool Tokenizer::isWhitespace(char p) {
	// Whitespace is either a newline, tab character or space
	if (p == '\n' || p == 13 || p == ' ' || p == '\t')
		return true;
	else
		return false;
}

/* Tokenizer::isSpecialCharacter
 * Checks if a character is a 'special' character, ie a character
 * that should always be its own token (;, =, | etc)
 *******************************************************************/
bool Tokenizer::isSpecialCharacter(char p) {
	// Check through special tokens string
	for (unsigned a = 0; a < special.size(); a++) {
		if (special[a] == p)
			return true;
	}

	return false;
}

/* Tokenizer::incrementCurrent
 * Increments the position pointer, returns false on end of block
 *******************************************************************/
bool Tokenizer::incrementCurrent() {
	if (position >= size - 1) {
		// At end of text, return false
		position = size;
		return false;
	} else {
		// Check for newline
		if (current[0] == '\n')
			line++;

		// Increment position & current pointer
		position++;
		current++;
		t_end++;
		return true;
	}
}

/* Tokenizer::skipLineComment
 * Skips a '//' comment
 *******************************************************************/
void Tokenizer::skipLineComment() {
	// Increment position until a newline or end is found
	while (current[0] != '\n' && current[0] != 13) {
		if (!incrementCurrent())
			return;
	}

	// Skip the newline character also
	incrementCurrent();
}

/* Tokenizer::skipMultilineComment
 * Skips a multiline comment (like this one :P)
 *******************************************************************/
void Tokenizer::skipMultilineComment() {
	// Increment position until '*/' or end is found
	while (!(current[0] == '*' && current[1] == '/')) {
		if (!incrementCurrent())
			return;
	}

	// Skip the ending '*/'
	incrementCurrent();
	incrementCurrent();
}

/* Tokenizer::getToken
 * Gets the next 'token' from the text & moves past it, returns
 * a blank string if we're at the end of the text
 *******************************************************************/
string Tokenizer::getToken() {
	string ret_str = "";
	bool ready = false;
	qstring = false;

	// Increment pointer to next token
	while (!ready) {
		ready = true;

		// Increment pointer until non-whitespace is found
		while (isWhitespace(current[0])) {
			// Return if end of text found
			if (!incrementCurrent())
				return ret_str;
		}

		// Skip C-style comments
		if (comments & CCOMMENTS) {
			// Check if we have a line comment
			if (current[0] == '/' && current[1] == '/' && current[2] != '$') {
				skipLineComment(); // Skip it
				ready = false;
			}

			// Check if we have a multiline comment
			if (current[0] == '/' && current[1] == '*') {
				skipMultilineComment(); // Skip it
				ready = false;
			}
		}

		// Skip '##' comments
		if (comments & HCOMMENTS) {
			if (current[0] == '#' && current[1] == '#') {
				skipLineComment(); // Skip it
				ready = false;
			}
		}

		// Skip ';' comments
		if (comments & SCOMMENTS) {
			if (current[0] == ';') {
				skipLineComment(); // Skip it
				ready = false;
			}
		}

		// Check for end of text
		if (position == size)
			return ret_str;
	}

	// Init token delimiters
	t_start = position;
	t_end = position;

	// If we're at a special character, it's our token
	if (isSpecialCharacter(current[0])) {
		ret_str += current[0];
		t_end = position + 1;
		incrementCurrent();
		return ret_str;
	}

	// Now read the token
	if (current[0] == '\"') { // If we have a literal string (enclosed with "")
		qstring = true;

		// Skip opening "
		incrementCurrent();

		// Read literal string (include whitespace)
		while (current[0] != '\"') {
			ret_str += current[0];

			if (!incrementCurrent())
				return ret_str;
		}

		// Skip closing "
		incrementCurrent();
	} else {
		// Read token (don't include whitespace)
		while (!isWhitespace(current[0])) {
			// Return if special character found
			if (isSpecialCharacter(current[0]))
				return ret_str;

			// Add current character to the token
			ret_str += current[0];

			// Return if end of text found
			if (!incrementCurrent())
				return ret_str;
		}
	}

	// Write token to log if debug mode enabled
	if (debug)
		wxLogMessage(ret_str);

	// Return the token
	return ret_str;
}

/* Tokenizer::peekToken
 * Returns the next token without actually moving past it
 *******************************************************************/
string Tokenizer::peekToken() {
	// Backup current position
	char* c = current;
	uint32_t p = position;

	// Read the next token
	string token = getToken();

	// Go back to original position
	current = c;
	position = p;

	// Return the token
	return token;
}

/* Tokenizer::checkToken
 * Compares the current token with a string
 *******************************************************************/
bool Tokenizer::checkToken(string check) {
	return !(getToken().Cmp(check));
}

/* Tokenizer::getInteger
 * Reads a token and returns its integer value
 *******************************************************************/
int Tokenizer::getInteger() {
	// Get token
	string token = getToken();

	// Return integer value
	return atoi(CHR(token));
}

/* Tokenizer::getFloat
 * Reads a token and returns its floating point value
 *******************************************************************/
float Tokenizer::getFloat() {
	// Get token
	string token = getToken();

	// Return float value
	return (float) atof(CHR(token));
}

/* Tokenizer::getDouble
 * Reads a token and returns its double-precision floating point
 * value
 *******************************************************************/
double Tokenizer::getDouble() {
	// Get token
	string token = getToken();

	// Return double value
	return atof(CHR(token));
}

/* Tokenizer::getBool
 * Reads a token and returns its boolean value, anything except
 * "0", "no", or "false" will return true
 *******************************************************************/
bool Tokenizer::getBool() {
	// Get token
	string token = getToken();

	// If the token is a string "no" or "false", the value is false
	if (S_CMPNOCASE(token, "no") || S_CMPNOCASE(token, "false"))
		return false;

	// Returns true ("1") or false ("0")
	return !!atoi(CHR(token));
}