/*
 * Parser.cpp
 *
 *  Created on: Jan 30, 2016
 *      Author: bobbey
 */

#include <cassert>
#include <fstream>
#include <iterator>
#include <sstream>
#include <algorithm>

#include "Command.hpp"
#include "Parser.hpp"
#include "Word.hpp"

BadCommandException::BadCommandException(const string& message) throw(){
	this->message = "Bad Command: " + message;
}

const char* BadCommandException::what() const throw() {
	return message.c_str();
}

DictionaryLoadException::DictionaryLoadException(const string& line) throw():
	line(line){}

const char* DictionaryLoadException::what() const throw() {
	return ("The line\n" + line + "\nis malformed.").c_str();
}

ParseResult::ParseResult(
	const ParseStatus status,
	const unsigned numParsed,
	const vector<Command>& commands
	): status(status), numParsed(numParsed), commands(commands){}

const pair<char, PartOfSpeech> partDecoderEntries[] = {
	pair<char, PartOfSpeech>('n', PartOfSpeech::NOUN),
	pair<char, PartOfSpeech>('v', PartOfSpeech::VERB),
	pair<char, PartOfSpeech>('a', PartOfSpeech::ADJECTIVE),
	pair<char, PartOfSpeech>('b', PartOfSpeech::ADVERB),
	pair<char, PartOfSpeech>('p', PartOfSpeech::PREPOSITION),
	pair<char, PartOfSpeech>('q', PartOfSpeech::QUANTIFIER)
};

const map<char, PartOfSpeech> partDecoder(partDecoderEntries, end(partDecoderEntries));

PartOfSpeech partOfSpeechFromChar(const char letter) throw(){
	if (partDecoder.find(letter) != partDecoder.end()) {
		return partDecoder.at(letter);
	}
	return PartOfSpeech::NOTHING;
}

ParseResult Parser::checkGrammar(const vector<vector<Word> >& wordOptions) throw() {
	vector<vector<Word> > validGrammars;
	vector<vector<Word> > newValidGrammars;
	vector<pair<GrammarStatus, PartOfSpeech> > lastResults;
	vector<pair<GrammarStatus, PartOfSpeech> > currentResults;
	ParseStatus failStatus;
	bool validGrammarPossible = true;
	unsigned i = 0;
	for (i = 0;i < wordOptions.size() && validGrammarPossible;++i) {
		validGrammarPossible = false;
		for (vector<Word>::const_iterator it = wordOptions[i].begin();it != wordOptions[i].end();++it) {
			newValidGrammars.clear();
			auto updateGrammar = [&](
				const PartOfSpeech lastPart,
				const GrammarStatus lastGrammarStatus,
				vector<Word> currentGrammar
				) {
				pair<GrammarStatus, ParseStatus> lastPair = checkSpeech(it->partOfSpeech, lastGrammarStatus, lastPart);
				if (
					i == wordOptions.size() - 1
					&& lastPair.second == ParseStatus::GOOD
					&& it->partOfSpeech != PartOfSpeech::NOUN
					&& it->partOfSpeech != PartOfSpeech::VERB
					&& it->partOfSpeech != PartOfSpeech::ADVERB
					) {
					lastPair.second = ParseStatus::INCOMPLETE;
				}
				if (lastPair.second == ParseStatus::GOOD) {
					currentResults.push_back(pair<GrammarStatus, PartOfSpeech>(lastPair.first, it->partOfSpeech));
					if (it->partOfSpeech != PartOfSpeech::ARTICLE && it->partOfSpeech != PartOfSpeech::AND) {
						currentGrammar.push_back(*it);
					} else {
						assert(wordOptions[i].size() == 1);
					}
					newValidGrammars.push_back(currentGrammar);
					validGrammarPossible = true;
				} else {
					failStatus = lastPair.second;
				}
			};
			if (i == 0){
				 updateGrammar(PartOfSpeech::NOTHING, GrammarStatus::DEFAULT, vector<Word>());
			} else {
				assert(lastResults.size() == validGrammars.size());
				assert(lastResults.size() != 0);
				assert(validGrammars[0].size() == i);
				for (unsigned j = 0;j < lastResults.size();++j) {
					updateGrammar(lastResults[j].second, lastResults[j].first, validGrammars[j]);
				}
			}
		}
		lastResults = currentResults;
		validGrammars = newValidGrammars;
		newValidGrammars.clear();
		currentResults.clear();
	}
	if (validGrammarPossible) {
		vector<Command> commands = makeCommands(validGrammars);
		return ParseResult(ParseStatus::GOOD, wordOptions.size(), commands);
	} else {
		return ParseResult(failStatus, i);
	}
}

pair<GrammarStatus, ParseStatus> Parser::checkSpeech(
	const PartOfSpeech partOfSpeech,
	GrammarStatus grammarStatus,
	const PartOfSpeech previousPart
	) throw() {
	ParseStatus parseStatus = ParseStatus::GOOD;
	switch(previousPart) {
		case PartOfSpeech::NOTHING:
			assert(!(grammarStatus & GrammarStatus::FOUND_VERB));
			assert(!(grammarStatus & GrammarStatus::FOUND_ADVERB));
			switch (partOfSpeech) {
				case PartOfSpeech::VERB:
					grammarStatus |= GrammarStatus::FOUND_VERB;
					break;
				case PartOfSpeech::ADVERB:
					grammarStatus |= GrammarStatus::FOUND_ADVERB;
					break;
				default:
					parseStatus = ParseStatus::BAD_GRAMMAR;
			}
			break;
		case PartOfSpeech::NOUN:
			switch (partOfSpeech) {
				case PartOfSpeech::ADVERB:
					if (!!(grammarStatus & GrammarStatus::FOUND_ADVERB)) {
						parseStatus = ParseStatus::MULTI_ADVERB;
					} else {
						grammarStatus |= GrammarStatus::FOUND_ADVERB;
					}
					break;
				case PartOfSpeech::PREPOSITION:
					break;
				case PartOfSpeech::AND:
					parseStatus = ParseStatus::MULTI_COMMAND;
					break;
				default:
					parseStatus = ParseStatus::BAD_GRAMMAR;
					break;
			}
			break;
		case PartOfSpeech::VERB:
			switch(partOfSpeech) {
				case PartOfSpeech::ADVERB:
					if (!!(grammarStatus & GrammarStatus::FOUND_ADVERB)) {
						parseStatus = ParseStatus::MULTI_ADVERB;
					} else {
						grammarStatus |= GrammarStatus::FOUND_ADVERB | GrammarStatus::ADVERB_AFTER_VERB;
					}
					break;
				case PartOfSpeech::AND:
					parseStatus = ParseStatus::MULTI_COMMAND;
					break;
				case PartOfSpeech::VERB:
					parseStatus = ParseStatus::BAD_GRAMMAR;
					break;
				default:
					break;
			}
			break;
		case PartOfSpeech::ADJECTIVE:
			switch(partOfSpeech) {
				case PartOfSpeech::NOUN:
				case PartOfSpeech::ADJECTIVE:
				case PartOfSpeech::AND:
					break;
				default:
					parseStatus = ParseStatus::BAD_GRAMMAR;
					break;
			}
			break;
		//Adverbs are allowed only at the beginning and end of a sentence and after a verb.
		case PartOfSpeech::ADVERB:
			if (!!(grammarStatus & GrammarStatus::ADVERB_AFTER_VERB)) {
				if (partOfSpeech != PartOfSpeech::PREPOSITION) {
					parseStatus = ParseStatus::BAD_GRAMMAR;
				}
			} else if (!!(grammarStatus & GrammarStatus::FOUND_VERB)) { //Adverb as last word.
				if (partOfSpeech == PartOfSpeech::AND) {
					parseStatus = ParseStatus::MULTI_COMMAND;
				} else {
					parseStatus = ParseStatus::BAD_GRAMMAR;
				}
			} else { //Adverb as first word.
				if (partOfSpeech == PartOfSpeech::VERB) {
					grammarStatus = GrammarStatus::FOUND_VERB;
				}
				else {
					parseStatus = ParseStatus::BAD_GRAMMAR;
				}
			}
			break;
		case PartOfSpeech::PREPOSITION:
			switch (partOfSpeech) {
				case PartOfSpeech::NOUN:
				case PartOfSpeech::ARTICLE:
				case PartOfSpeech::QUANTIFIER:
				case PartOfSpeech::A:
					break;
				default:
					parseStatus = ParseStatus::BAD_GRAMMAR;
					break;
			}
			break;
		case PartOfSpeech::ARTICLE:
			switch (partOfSpeech) {
				case PartOfSpeech::NOUN:
				case PartOfSpeech::ADJECTIVE:
				case PartOfSpeech::QUANTIFIER:
					break;
				default:
					parseStatus = ParseStatus::BAD_GRAMMAR;
					break;
			}
			break;
		case PartOfSpeech::QUANTIFIER:
		case PartOfSpeech::A:
			switch (partOfSpeech) {
				case PartOfSpeech::NOUN:
				case PartOfSpeech::ADJECTIVE:
					break;
				default:
					parseStatus = ParseStatus::BAD_GRAMMAR;
					break;
			}
			break;
		case PartOfSpeech::AND:
			if (partOfSpeech != PartOfSpeech::ADJECTIVE) {
				parseStatus = ParseStatus::BAD_GRAMMAR;
			}
			break;
	}
	return pair<GrammarStatus, ParseStatus>(grammarStatus, parseStatus);
}

Command Parser::makeCommand(const vector<Word>& words) {
	string verb, adverb, preposition, quantifier;
	vector<string> adjectives;
	Thing directObject;
	vector<Thing> indirectObjects;
	Thing currentObject;
	bool foundObject;
	for(unsigned i = 0;i < words.size();++i) {
		switch (words[i].partOfSpeech) {
			case PartOfSpeech::VERB:
				verb = words[i].name;
				break;
			case PartOfSpeech::ADVERB:
				adverb = words[i].name;
				break;
			case PartOfSpeech::NOUN:
				currentObject = Thing(words[i].name, preposition, quantifier, adjectives);
				adjectives.clear();
				if (foundObject) {
					indirectObjects.push_back(currentObject);
				} else {
					directObject = currentObject;
					foundObject = true;
				}
				break;
			case PartOfSpeech::ADJECTIVE:
				adjectives.push_back(words[i].name);
				break;
			case PartOfSpeech::PREPOSITION:
				preposition = words[i].name;
				break;
			case PartOfSpeech::QUANTIFIER:
			case PartOfSpeech::A:
				quantifier = words[i].name;
				break;
			default:
				string message = "";
				for (vector<Word>::const_iterator it = words.begin();it != words.end();++it) {
					message += it->name;
				}
				throw BadCommandException(message);
		}
	}
	Action action(verb, adverb);
	return Command(action, directObject, indirectObjects);
}

vector<Command> Parser::makeCommands(const vector<vector<Word> >& wordOptions) {
	vector<Command> commands;
	for (vector<vector<Word> >::const_iterator it = wordOptions.begin();it != wordOptions.end();++it) {
		commands.push_back(makeCommand(*it));
	}
	return commands;
}

vector<string> Parser::split(const string& input) throw() {
	vector<string> splits;
	string current;
	unsigned i = 0;
	while (i < input.length()) {
		while (i < input.length() && (input[i] == ' ' || input[i] == '\t')) {
			++i;
		}
		if (i < input.length()) {
			while (i < input.length() && !(input[i] == ' ' || input[i] == '\t')) {
				current += input[i++];
			}
			splits.push_back(current);
		}
	}
	return splits;
}

string Parser::toLower(const string& input) throw() {
	string lowerCased;
	for (unsigned i = 0; i < input.length();++i) {
		if (input[i] >= 'A' && input[i] <= 'Z') {
			lowerCased.push_back(input[i] + 'a' - 'A');
		} else {
			lowerCased.push_back(input[i]);
		}
	}
	return lowerCased;
}

bool Parser::addEntry(const string& key, const Word& word) throw() {
	if (!hasKey(key)) {
		dictionary.insert(pair<string, vector<Word> >(key, vector<Word>()));
	}
	if (!hasEntry(key, word)) {
		vector<Word> entryVector;
		entryVector.push_back(word);
		dictionary[key] = entryVector;
		return true;
	}
	return false;
}

bool Parser::hasEntry(const string& key, const Word& word) const throw() {
	if (hasKey(key)) {
		return find(dictionary.at(key).begin(), dictionary.at(key).end(), word) != dictionary.at(key).end();
	}
	return false;
}

bool Parser::hasKey(const string& key) const throw() {
	return dictionary.find(key) != dictionary.end();
}

void Parser::loadDictionary(istream* const input) {
	stringstream lineStream;
	string line;
	string principle;
	string last;
	Word word;
	PartOfSpeech partOfSpeech;
	while (!input->eof()) {
		lineStream.clear();
		getline(*input, line);
		lineStream << line;
		lineStream >> principle;
		lineStream.ignore(1);
		word = Word(principle, partOfSpeech);
		partOfSpeech = partOfSpeechFromChar((char)lineStream.get());
		if (lineStream.fail()) {
			throw DictionaryLoadException(line);
		}
		last = principle;
		while(!lineStream.fail()) {
			addEntry(last, word);
			lineStream >> last;
		}
	}
}

void Parser::loadDictionary(const string& inputPath) {
	ifstream fileStream(inputPath);
	loadDictionary(&fileStream);
}

ParseResult Parser::parse(const vector<string>& input) const {
	vector<vector<Word> > wordOptions;
	for (unsigned i = 0;i < input.size();++i) {
		if (hasKey(input[i])) {
			wordOptions.push_back(dictionary.at(input[i]));
		} else {
			return ParseResult(ParseStatus::UNKNOWN_WORD, i);
		}
	}
	return checkGrammar(wordOptions);
}

bool Parser::removeEntry(const string& key, const Word& word) throw() {
	if (hasKey(key)) {
		vector<Word>::iterator it = find(dictionary[key].begin(), dictionary[key].end(), word);
		if (it != dictionary[key].end()) {
			dictionary[key].erase(it);
			if (dictionary[key].size() == 0) {
				dictionary.erase(key);
			}
			return true;
		}
	}
	return false;
}
