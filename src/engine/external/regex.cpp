#include "regex.h"

#include <regex>

class Regex::Data {
public:
	std::regex regex;
	std::string error;
};

Regex::Regex(const Regex& other) : data(new Data(*other.data)) {}

Regex::Regex(Regex&& Other) : data(Other.data) {
	Other.data = nullptr;
}

Regex::Regex() : data(new Data{{}, "Empty"}) {}

Regex::Regex(const std::string& pattern) : data(new Data) {
	try {
		data->regex = std::regex(pattern);
		data->error = "";
	} catch (const std::regex_error& e) {
		data->error = e.what();
	}
}

Regex& Regex::operator=(const Regex& other) {
	if (this != &other) {
		delete data;
		data = new Data(*other.data);
	}
	return *this;
}

Regex& Regex::operator=(Regex&& other) {
	if (this != &other) {
		delete data;
		data = other.data;
		other.data = nullptr;
	}
	return *this;
}

Regex::~Regex() {
	delete data;
}

std::string Regex::error() const {
	return data->error;
}

bool Regex::match(const char* str) {
	if (!data->error.empty())
		return false;
	return std::regex_match(str, data->regex);
}
