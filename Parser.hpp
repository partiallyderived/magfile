/*
 * Parser.hpp
 *
 *  Created on: Jan 30, 2016
 *      Author: bobbey
 */

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <exception>
#include <string>
#include <istream>
#include <vector>
#include <map>

#include "Utilities.hpp"
#include "Word.hpp"

class Command;
class Game;
class Parser;

using namespace std;

enum class GrammarStatus {
	DEFAULT = 0,
	FOUND_VERB = 1,
	FOUND_ADVERB = 2,
	ADVERB_AFTER_VERB = 4,
	ALL = 7
};

DEFINE_FLAG_OVERLOADS(GrammarStatus)

PartOfSpeech partOfSpeechFromChar(const char letter) throw();

class ParserException: public exception {};

class BadCommandException: public ParserException {
	private:
		string message;

	public:
		BadCommandException(const string& message) throw();

		const char* what() const throw();
};

class DictionaryLoadException: public ParserException {
	private:
		const string line;

	public:
		DictionaryLoadException(const string& line) throw();

		const char* what() const throw();
};

enum class ParseStatus {
	GOOD = 0,
	UNKNOWN_WORD,
	MULTI_ADVERB,
	MULTI_COMMAND,
	BAD_GRAMMAR,
	INCOMPLETE
};

class ParseResult {
	friend Game;
	friend Parser;
	private:
		ParseStatus status;
		unsigned numParsed;
		vector<Command> commands;

	public:
		ParseResult(
			const ParseStatus status = ParseStatus::GOOD,
			const unsigned numParsed = 0,
			const vector<Command>& commands = vector<Command>()
			);
};

class Parser {
	private:
		static ParseResult checkGrammar(const vector<vector<Word> >& wordOptions) throw();

		static pair<GrammarStatus, ParseStatus> checkSpeech(
			const PartOfSpeech partOfSpeech,
			GrammarStatus grammarStatus,
			const PartOfSpeech previousPart
			) throw();

		static Command makeCommand(const vector<Word>& words);
		static vector<Command> makeCommands(const vector<vector<Word> >& wordOptions);

		map<string, vector<Word> > dictionary;

		vector<vector<Word> > getWordOptions(const vector<string>& input) const throw();

	public:
		static vector<string> split(const string& input) throw();
		static string toLower(const string& input) throw();

		bool addEntry(const string& key, const Word& word) throw();
		vector<Word> getWords(const string& key) const throw();
		bool hasEntry(const string& key, const Word& word) const throw();
		bool hasKey(const string& key) const throw();
		void loadDictionary(istream* const input);
		void loadDictionary(const string& inputPath);
		ParseResult parse(const vector<string>& input) const;
		bool removeEntry(const string& key, const Word& word) throw();

};

#endif /* PARSER_HPP_ */
