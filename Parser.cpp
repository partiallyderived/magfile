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

#include "Parser.hpp"
#include "Word.hpp"

BadCommandException::BadCommandException(const vector<Word>& words) throw(){
	message = "Bad Command: ";
	for(vector<Word>::iterator it = words.begin();it != words.end();++it) {
		message += it->getName();
	}
}

const char* BadCommandException::what() const throw() {
	return message.c_str();
}

DictionaryLoadException::DictionaryLoadException(const string& line):
	line(line){}

const char* DictionaryLoadException::what() const throw() {
	return ("The line\n" + line + "\nis malformed.").c_str();
}

Action::Action(const string& verb, const string& adverb) throw(): verb(verb), adverb(adverb){}

string Action::getAdverb() const throw() {
	return adverb;
}

string Action::getVerb() const throw() {
	return verb;
}

bool Action::hasAdverb() const throw() {
	return adverb.length() != 0;
}

Item Item::NOTHING("");

Item::Item(const string& name, const string& preposition, const string& quantifier, const vector<string>& adjectives) throw():
	name(name), preposition(preposition), quantifier(quantifier), adjectives(adjectives){}

vector<string> Item::getAdjectives() const throw() {
	return adjectives;
}

string Item::getName() const throw() {
	return name;
}

string Item::getQuantifier() const throw() {
	return quantifier;
}

bool Item::hasAdjectives() const throw() {
	return !adjectives.empty();
}

bool Item::hasQuantifier() const throw() {
	return quantifier != "";
}

Command::Command(const Action& action, const Item& directObject, const vector<Item>& indirectObjects) throw():
	action(action),
	directObject(directObject),
	indirectObjects(indirectObjects){}

bool Command::hasIndirectObjects() const throw(){
	return !indirectObjects.empty();
}

bool Command::hasDirectObject() const throw(){
	return directObject != Item::NOTHING;
}

Action Command::getAction() const throw(){
	return action;
}

vector<Item> Command::getIndirectObjects() const throw(){
	return indirectObjects;
}

Item Command::getDirectObject() const throw(){
	return directObject;
}

pair<GrammarStatus, unsigned> Parser::checkGrammar(vector<vector<Word> >* const wordOptions) throw() {
	GrammarStatus status = GrammarStatus::DEFAULT;
	return checkGrammarRecurse(wordOptions, 0, status, PartOfSpeech::NOTHING);
}

pair<GrammarStatus, unsigned> Parser::checkGrammarRecurse(vector<vector<Word> >* wordOptions,
	const unsigned i,
	const GrammarStatus status,
	const PartOfSpeech previousPart
	) {

	pair<GrammarStatus, unsigned> bestPair(status, i);
	if (i == wordOptions->size()) {
		return bestPair;
	}
	GrammarStatus lastStatus, bestStatus;
	pair<GrammarStatus, unsigned> lastPair;
	for (vector<Word>::iterator it = (*wordOptions)[i].begin();it != (*wordOptions)[i].end();) {
		lastStatus = checkWord(*it, status, previousPart);
		if (i == wordOptions->size() - 1) {
			if (!(lastStatus & GrammarStatus::FOUND_VERB) ||
				!(it->getPartOfSpeech() & (PartOfSpeech::VERB | PartOfSpeech::NOUN | PartOfSpeech::ADVERB))){
				//Commands should end with either a noun, a verb, or an adverb and a verb must be present.
				lastStatus |= GrammarStatus::INCOMPLETE;
			}
		}
		if (lastStatus & GrammarStatus::FAIL) {
			it = (*wordOptions)[i].erase(it);

		} else {
			lastPair = checkGrammarRecurse(wordOptions, i + 1, lastStatus, it->getPartOfSpeech());
			if (lastPair.second > bestPair.second) {
				bestPair = lastPair;
			}
			++it;
		}
	}
	return bestPair;
}

GrammarStatus Parser::checkWord(const Word& word, GrammarStatus status, const PartOfSpeech previousPart) throw(){
	switch(previousPart) {
		case PartOfSpeech::NOTHING:
			assert(!(status & GrammarStatus::FOUND_VERB));
			assert(!(status & GrammarStatus::FOUND_ADVERB));
			switch (word.getPartOfSpeech()) {
				case PartOfSpeech::VERB:
					status |= GrammarStatus::FOUND_VERB;
					break;
				case PartOfSpeech::ADVERB:
					status |= GrammarStatus::FOUND_ADVERB;
					break;
				default:
					status |= GrammarStatus::BAD_GRAMMAR;
			}
			break;
		case PartOfSpeech::NOUN:
			switch (word.getPartOfSpeech()) {
				case PartOfSpeech::ADVERB:
					if (status & GrammarStatus::FOUND_ADVERB) {
						status |= GrammarStatus::MULTI_ADVERB;
					} else {
						status |= GrammarStatus::FOUND_ADVERB;
					}
					break;
				case PartOfSpeech::PREPOSITION:
					break;
				case PartOfSpeech::AND:
					status |= GrammarStatus::MULTI_COMMAND;
					break;
				default:
					status |= GrammarStatus::BAD_GRAMMAR;
					break;
			}
			break;
		case PartOfSpeech::VERB:
			switch(word.getPartOfSpeech()) {
				case PartOfSpeech::ADVERB:
					if (status & GrammarStatus::FOUND_ADVERB) {
						status |= GrammarStatus::MULTI_ADVERB;
					} else {
						status |= GrammarStatus::FOUND_ADVERB | GrammarStatus::ADVERB_AFTER_VERB;
					}
					break;
				case PartOfSpeech::AND:
					status |= GrammarStatus::MULTI_COMMAND;
					break;
				case PartOfSpeech::VERB:
					status |= GrammarStatus::BAD_GRAMMAR;
					break;
			}
			break;
		case PartOfSpeech::ADJECTIVE:
			switch(word.getPartOfSpeech()) {
				case PartOfSpeech::NOUN:
				case PartOfSpeech::ADJECTIVE:
				case PartOfSpeech::AND:
					break;
				default:
					status |= GrammarStatus::BAD_GRAMMAR;
					break;
			}
			break;
		//Adverbs are allowed only at the beginning and end of a sentence and after a verb.
		case PartOfSpeech::ADVERB:
			if (status & GrammarStatus::ADVERB_AFTER_VERB) {
				if (word.getPartOfSpeech() != PartOfSpeech::PREPOSITION) {
					status |= GrammarStatus::BAD_GRAMMAR;
				}
			} else if (status & GrammarStatus::FOUND_VERB) { //Adverb as last word.
				if (word.getPartOfSpeech() == PartOfSpeech::AND) {
					status |= GrammarStatus::MULTI_COMMAND;
				} else {
					status |= GrammarStatus::BAD_GRAMMAR;
				}
			} else { //Adverb as first word.
				if (word.getPartOfSpeech() == PartOfSpeech::VERB) {
					status |= GrammarStatus::FOUND_VERB;
				}
				else {
					status |= GrammarStatus::BAD_GRAMMAR;
				}
			}
			break;
		case PartOfSpeech::PREPOSITION:
			switch (word.getPartOfSpeech()) {
				case PartOfSpeech::NOUN:
				case PartOfSpeech::ARTICLE:
				case PartOfSpeech::QUANTIFIER:
				case PartOfSpeech::A:
					break;
				default:
					status |= GrammarStatus::BAD_GRAMMAR;
					break;
			}
			break;
		case PartOfSpeech::ARTICLE:
			switch (word.getPartOfSpeech()) {
				case PartOfSpeech::NOUN:
				case PartOfSpeech::ADJECTIVE:
				case PartOfSpeech::QUANTIFIER:
					break;
				default:
					status |= GrammarStatus::BAD_GRAMMAR;
					break;
			}
			break;
		case PartOfSpeech::QUANTIFIER:
		case PartOfSpeech::A:
			switch (word.getPartOfSpeech()) {
				case PartOfSpeech::NOUN:
				case PartOfSpeech::ADJECTIVE:
					break;
				default:
					status |= GrammarStatus::BAD_GRAMMAR;
					break;
			}
			break;
		case PartOfSpeech::AND:
			if (word.getPartOfSpeech() != PartOfSpeech::ADJECTIVE) {
				status |= GrammarStatus::BAD_GRAMMAR;
			}
			break;
	}
	return status;
}

vector<vector<Word> > Parser::expandSentences(const vector<vector<Word> >& wordOptions) throw() {
	vector<vector<Word> > sentences, lastSentences;
	sentences.push_back(vector<Word>());
	vector<Word> newVector;
	for(vector<vector<Word> >::iterator it = wordOptions.iterator();it != wordOptions.end();++it) {
		lastSentences = sentences;
		sentences = vector<vector<Word> >();
		for(vector<Word>::iterator jt = it->iterator();jt != it->end();++jt) {
			for(vector<Word>::iterator kt = lastSentences.iterator();kt != lastSentences.end();++kt) {
				newVector = *kt;
				newVector.push_back(*jt);
				sentences.push_back(newVector);
			}
		}

	}
	return sentences;
}

void Parser::filter(vector<vector<Word> >* const wordOptions) {
	for (vector<vector<Word> >::iterator it = wordOptions->iterator();it != wordOptions->end();){
		if ((*it)[0].getPartOfSpeech() == PartOfSpeech::ARTICLE || (*it)[0].getName() == "and") {
			it = wordOptions->erase(it);
		} else {
			++it;
		}
	}
}

int Parser::find(const Word& word, const vector<Word>& words) throw() {
	for (int i = 0;i < words.size();++i) {
		if (word == words[i]) {
			return i;
		}
	}
	return -1;
}

Command Parser::makeCommand(const vector<Word>& words) throw() {
	string verb, adverb, preposition, quantifier;
	vector<string> adjectives;
	Item directObject;
	vector<Item> indirectObjects;
	Item currentObject;
	bool foundObject;
	for(vector<Word>::iterator it = words.begin();it != words.end();++it) {
		switch (it->getPartOfSpeech) {
			case PartOfSpeech::VERB:
				verb = it->getName();
				break;
			case PartOfSpeech::ADVERB:
				adverb = it->getName();
				break;
			case PartOfSpeech::NOUN:
				currentObject = Item(it->getName(), preposition, quantifier, adjectives);
				adjectives.clear();
				if (foundObject) {
					indirectObjects.push_back(currentObject);
				} else {
					directObject = currentObject;
					foundObject = true;
				}
				break;
			case PartOfSpeech::ADJECTIVE:
				adjectives.push_back(it->getName());
				break;
			case PartOfSpeech::PREPOSITION:
				preposition = it->getName();
				break;
			case PartOfSpeech::QUANTIFIER:
			case PartOfSpeech::A:
				quantifier = it->getName();
				break;
			default:
				throw BadCommandException(words);
		}
	}
	Action action(verb, adverb);
	return Command(action, directObject, indirectObjects);
}

vector<Command> Parser::makeCommands(const vector<vector<Word> >& wordOptions) {
	vector<vector<Word> > sentences = expandSentences(wordOptions);
	vector<Command> commands;
	for (vector<vector<Word> >::iterator it = sentences.begin();it != sentences.end();++it) {
		commands.push_back(makeCommand(*it));
	}
	return commands;
}

vector<string> Parser::split(const string& input) throw() {
	vector<string> splits;
	string current;
	int i = 0;
	while (i < input.length()) {
		while (input[i] == ' ' || input[i] == '\t')
			++i;
		if (i<input.length) {
			while (!(input[i] == ' ' || input[i] == '\t'))
				current += input[i++];
			splits.push_back(current);
		}
	}
	return splits;
}

string Parser::toLower(const string& input) throw() {
	string lowerCased;
	for (int i = 0; i < input.length();++i) {
		if (input[i] >= 'A' && input[i] <= "Z") {
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
		dictionary[key].push_back(word);
		return true;
	}
	return false;
}

vector<Word> Parser::getWords(const string& key) const throw() {
	return hasKey(key) ? dictionary[key]: vector<Word>();
}

bool Parser::hasEntry(const string& key, const Word& word) const throw() {
	if (hasKey(key)) {
		return find(word, dictionary[key]) != -1;
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
	PartOfSpeech partOfSpeech;
	while (!input->eof()) {
		lineStream.clear();
		getline(*input, line);
		lineStream << line;
		lineStream >> principle;
		lineStream.ignore(1);
		partOfSpeech = partOfSpeechFromChar((char)lineStream.get());
		if (lineStream.fail()) {
			throw DictionaryLoadException(line);
		}
		Word word = Word(principle, partOfSpeech);
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

vector<vector<Word> > Parser::parseWordOptions(const string& input) const throw() {
	vector<string> splits = split(input);
	vector<vector<Word> > wordOptions;
	for (int i = 0;i < splits.size();++i) {
		if (hasKey(splits[i])) {
			wordOptions.push_back(dictionary[splits[i]]);
		} else {
			wordOptions.push_back(vector<Word>());
		}
	}
	return wordOptions;
}

bool Parser::removeEntry(const string& key, const Word word) throw() {
	if (hasKey(key)) {
		int i = find(word, dictionary[key]);
		if (i != -1) {
			dictionary[key].erase(dictionary[key].begin() + i);
			if (dictionary[key].size() == 0) {
				dictionary.erase(key);
			}
			return true;
		}
	}
	return false;
}
