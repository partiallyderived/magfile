/*
 * Word.hpp
 *
 *  Created on: Feb 20, 2016
 *      Author: bobbey
 */

#ifndef WORD_HPP_
#define WORD_HPP_

#include <string>

using namespace std;

class Game;
class Parser;

enum class PartOfSpeech {
		NOTHING,
		NOUN,
		VERB ,
		ADJECTIVE,
		ADVERB,
		PREPOSITION,
		ARTICLE,
		QUANTIFIER,
		A,
		AND
};

class Word {
	friend Game;
	friend Parser;
	private:
		string name;
		PartOfSpeech partOfSpeech;
	public:
		Word(const string& name = "", const PartOfSpeech partOfSpeech = PartOfSpeech::NOTHING);

		bool operator == (const Word& that) const throw();
};



#endif /* WORD_HPP_ */
