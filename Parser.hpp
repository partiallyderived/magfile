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

class Word;

using namespace std;

enum class GrammarStatus {
	DEFAULT = 0,
	FOUND_VERB = 1,
	FOUND_ADVERB = 2,
	ADVERB_AFTER_VERB = 4,
	MULTI_ADVERB = 8,
	MULTI_COMMAND = 16,
	BAD_GRAMMAR = 32,
	INCOMPLETE = 64,
	FAIL = MULTI_ADVERB | MULTI_COMMAND | BAD_GRAMMAR | INCOMPLETE
};

class Action {
	private:
		string verb, adverb;

	public:
		Action(const string& verb, const string& adverb = "") throw();

		string getAdverb() const throw();
		string getVerb() const throw();
		bool hasAdverb() const throw();

};

class Item {
	private:
		string name, preposition, quantifier;
		vector<string> adjectives;

	public:
		static const Item NOTHING;

		Item(const string& name = "",
			const string& preposition = "",
			const string& quantifier = "",
			const vector<string>& adjectives = vector<string>()) throw();

		vector<string> getAdjectives() const throw();
		string getName() const throw();
		string getPreposition() const throw();
		string getQuantifier() const throw();
		bool hasAdjectives() const throw();
		bool hasPreposition() const throw();
		bool hasQuantifier() const throw();

};

class Command {
	private:
		Action action;
		Item directObject;
		vector<Item> indirectObjects;

	public:
		Command(const Action& action, const Item& directObject = Item::NOTHING, const vector<Item>& indirectObjects = vector<Item>()) throw();

		bool hasIndirectObjects() const throw();
		bool hasDirectObject() const throw();
		Action getAction() const throw();
		vector<Item> getIndirectObjects() const throw();
		Item getDirectObject() const throw();
};

class ParserException: public exception {};

class BadCommandException: public ParserException {
	private:
		string message;

	public:
		BadCommandException(const vector<Word>& words) throw();

		const char* what() const throw();
};

class DictionaryLoadException: public ParserException {
	private:
		const string line;

	public:
		DictionaryLoadException(const string& line) throw();

		const char* what() const throw();
};

class Parser {
	private:
		static pair<GrammarStatus, unsigned> checkGrammarRecurse(vector<vector<Word> >* wordOptions,
			const unsigned i,
			const GrammarStatus status,
			const PartOfSpeech previousPart
		) throw();

		static pair<GrammarStatus, unsigned> checkGrammarRecurse(vector<vector<Word> >* wordOptions,
			const unsigned i,
			const GrammarStatus status,
			const PartOfSpeech previousPart
		);

		static GrammarStatus checkWord(const Word& word,
			GrammarStatus status,
			const PartOfSpeech previousPart) throw();

		static vector<vector<Word> > expandSentences(const vector<vector<Word> >& wordOptions) throw();

		static int find(const Word& word, const vector<Word>& words) throw();
		static vector<Command> makeCommandsRecurse(const vector<Word>& wordOptions, const unsigned i) throw();
		static vector<string> split(const string& input) throw();
		static string toLower(const string& input) throw();

		map<string, vector<Word> > dictionary;

	public:
		static pair<GrammarStatus, unsigned> checkGrammar(vector<vector<Word> >* const wordOptions) throw();
		static void filter(vector<vector<Word> >* wordOptions) throw();
		static Command makeCommand(const vector<Word>& words) throw();
		static vector<Command> makeCommands(const vector<vector<Word> >& wordOptions) throw();

		bool addEntry(const string& key, const Word& word) throw();
		vector<Word> getWords(const string& key) const throw();
		vector<vector<Word> > parseWordOptions(const string& input) const throw();
		bool hasEntry(const string& key, const Word& word) const throw();
		bool hasKey(const string& key) const throw();
		void loadDictionary(istream* const input);
		void loadDictionary(const string& inputPath);
		bool removeEntry(const string& key, const Word word) throw();
};

#endif /* PARSER_HPP_ */
