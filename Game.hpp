/*
 * Game.hpp
 *
 *  Created on: Jan 30, 2016
 *      Author: bobbey
 */

#ifndef GAME_HPP_
#define GAME_HPP_

#include <map>
#include <string>

#include "Parser.hpp"

typedef void (Game::*Command)();

using namespace std;

class Game {
	private:
		map<string, Command> commandMap;
	public:
		void handle(const string& message);

};

#endif /* GAME_HPP_ */
