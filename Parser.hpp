/*
 * Parser.hpp
 *
 *  Created on: Jan 30, 2016
 *      Author: bobbey
 */

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <string>
#include <vector>

#include "Game.hpp"

using namespace std;

class Parser {
	private:
		static const map<string, string> REDUCTION_MAP;

		static map<string, string> makeReductionMap();
		static vector<string> split(const string& input);
		static string toLower(const string& input);
	public:
		vector<string> parse(const string& input);
};



#endif /* PARSER_HPP_ */
