/*
 * Message.hpp
 *
 *  Created on: Jan 30, 2016
 *      Author: bobbey
 */

#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include <string>

using namespace std;

class Message {
	public:
		const string name;
		const string* const aliases;
		const int numAliases;

		template<int N>
		Message(const string& name, const string (&aliases)[N]): name(name), aliases(aliases), numAliases(N) {}
		Message(const string& name): name(name), aliases(&this->name), numAliases(1){}
};

const string NOT_FOUND = "not_found";

const string LOOK_ALIASES[] = {"l", "look", "see"};
const Message LOOK("l", LOOK_ALIASES);

const Message GRASS("grass");

const Message MESSAGES[] = {LOOK, GRASS};

#endif /* MESSAGE_HPP_ */
