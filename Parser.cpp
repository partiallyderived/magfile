/*
 * Parser.cpp
 *
 *  Created on: Jan 30, 2016
 *      Author: bobbey
 */

#include "Message.hpp"
#include "Parser.hpp"

const map<string, string> Parser::REDUCTION_MAP(Parser::makeReductionMap());

map<string, string> Parser::makeReductionMap() {
	map<string, string> tempMap;
	for(int i = 0;i < sizeof(MESSAGES)/sizeof(Message);++i) {
		for(int j = 0;j < MESSAGES[i].numAliases; ++j) {
			tempMap.insert(pair<string, string>(MESSAGES[i].aliases[j], MESSAGES[i].name));
		}
	}
	return tempMap;
}

vector<string> Parser::split(const string& input) {
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

string Parser::toLower(const string& input) {
	string lowerCased;
	for (int i = 0; i < input.length();++i) {
		if (input[i] < 'a') {
			lowerCased.push_back(input[i] + 'a' - 'A');
		}
		else {
			lowerCased.push_back(input[i]);
		}
	}
	return lowerCased;
}

vector<string> Parser::parse(const string& input) {
	vector<string> splits = split(input);
	vector<string> messages;
	for (int i = 0; i < splits.size();++i) {
		map<string, string>::const_iterator it = REDUCTION_MAP.find(toLower(splits[i]));
		if (it == REDUCTION_MAP.end()) {
			messages.push_back(NOT_FOUND);
		} else {
			messages.push_back(it->second);
		}
	}
	return messages;
}
