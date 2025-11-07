#pragma once

// Small CPP regex wrapper

#include <string>

class Regex {
private:
	class Data;
	Data* data;

public:
	Regex(const Regex& other); // Copy
	Regex(Regex&& other); // Move
	Regex& operator=(const Regex& other); // Copy
	Regex& operator=(Regex&& other); // Move

	Regex();
	Regex(const std::string& pattern);
	~Regex();

	std::string error() const;
	bool match(const char* str);
};
