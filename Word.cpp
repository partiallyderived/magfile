/*
 * Word.cpp
 *
 *  Created on: Feb 6, 2016
 *      Author: bobbey
 */

#include <map>

#include "Word.hpp"

Word Word::A("a", PartOfSpeech::A);
Word Word::AND("and", PartOfSpeech::AND);

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
		return partDecoder[letter];
	}
	return PartOfSpeech::NOTHING;
}

const Word Word::NOT_FOUND("not_found", PartOfSpeech::NOTHING);

Word::Word(const string& name, const PartOfSpeech partOfSpeech) throw()
	: name(name), partOfSpeech(partOfSpeech){}

string Word::getName() const throw() {
	return name;
}

PartOfSpeech Word::getPartOfSpeech() const throw() {
	return partOfSpeech;
}
