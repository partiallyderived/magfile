/*
 * Word.cpp
 *
 *  Created on: Feb 21, 2016
 *      Author: bobbey
 */

#include "Word.hpp"

Word::Word(const string& name, const PartOfSpeech partOfSpeech)
	: name(name), partOfSpeech(partOfSpeech){}

bool Word::operator ==(const Word& that) const throw() {
	return name == that.name && partOfSpeech == that.partOfSpeech;
}
