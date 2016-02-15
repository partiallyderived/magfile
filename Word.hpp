/*
 * Word.hpp
 *
 *  Created on: Feb 6, 2016
 *      Author: bobbey
 */

#ifndef WORD_HPP_
#define WORD_HPP_

#include <string>

using namespace std;

enum class PartOfSpeech {NOTHING = 0, NOUN, VERB, ADJECTIVE, ADVERB, PREPOSITION, ARTICLE, QUANTIFIER, A, AND};

PartOfSpeech partOfSpeechFromChar(const char letter) throw();

class Word {
	private:
		static const Word A, AND;

		string name;
		PartOfSpeech partOfSpeech;

	public:
		static const Word NOT_FOUND;

		Word(const string& name, const PartOfSpeech partOfSpeech) throw();

		string getName() const throw();

		PartOfSpeech getPartOfSpeech() const throw();
};

#endif /* WORD_HPP_ */
