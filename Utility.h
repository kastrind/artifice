#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class Utility {

	public:

		// Function to replace a line in a file
		static void replaceLineInFile(const std::string& filename, int lineNumber, const std::string& newContent) {
		std::ifstream fileIn(filename);
		if (!fileIn) {
			std::cerr << "Cannot open file for reading\n";
			return;
		}

		std::vector<std::string> lines;
		std::string line;

		// Read all lines into memory
		while (std::getline(fileIn, line)) {
			lines.push_back(line);
		}
		fileIn.close();

		// Check if line number is valid
		if (lineNumber < 0 || lineNumber >= lines.size()) {
			std::cerr << "Line number out of range\n";
			return;
		}

		// Replace the line
		lines[lineNumber] = newContent;

		// Write back all lines
		std::ofstream fileOut(filename);
		if (!fileOut) {
			std::cerr << "Cannot open file for writing\n";
			return;
		}

		for (const auto& l : lines) {
			fileOut << l << "\n";
		}
	}
};
