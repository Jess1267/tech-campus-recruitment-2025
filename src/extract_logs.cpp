#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

const std::string OUTPUT_DIR = "output";

// Function to perform binary search for the first occurrence of the date
std::streampos binarySearchFirst(const std::string &logFile, const std::string &targetDate) {
    std::ifstream file(logFile, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error: Unable to open file " << logFile << "\n";
        return -1;
    }

    std::streampos left = 0, right = file.tellg(), mid, result = -1;
    const size_t bufferSize = 1024 * 1024; // 1 MB buffer
    char buffer[bufferSize];

    while (left <= right) {
        mid = (left + right) / 2;
        file.seekg(mid);

        // Read buffer from the middle position
        file.read(buffer, bufferSize);
        std::string chunk(buffer, file.gcount());

        size_t firstNewLine = chunk.find('\n');
        if (firstNewLine != std::string::npos) {
            chunk = chunk.substr(firstNewLine + 1); // Skip partial line
        }

        size_t lastNewLine = chunk.rfind('\n');
        if (lastNewLine != std::string::npos) {
            chunk = chunk.substr(0, lastNewLine); // Remove trailing partial line
        }

        // Split into lines and check
        size_t pos = 0;
        std::string firstLine;
        while ((pos = chunk.find('\n')) != std::string::npos) {
            firstLine = chunk.substr(0, pos);
            break;
        }

        if (firstLine.compare(0, targetDate.size(), targetDate) >= 0) {
            result = mid;
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    return result;
}

void filterLogsByDate(const std::string &logFile, const std::string &targetDate) {
    if (!fs::exists(logFile)) {
        std::cerr << "Error: File not found: " << logFile << "\n";
        return;
    }

    if (!fs::exists(OUTPUT_DIR)) {
        fs::create_directories(OUTPUT_DIR);
    }

    std::string outputFileName = OUTPUT_DIR + "/output_" + targetDate + ".txt";
    std::ofstream outputFile(outputFileName);
    if (!outputFile) {
        std::cerr << "Error: Unable to create output file\n";
        return;
    }

    std::streampos startPos = binarySearchFirst(logFile, targetDate);
    if (startPos == -1) {
        std::cerr << "Error: No logs found for date " << targetDate << "\n";
        return;
    }

    std::ifstream file(logFile);
    file.seekg(startPos);
    std::string line;

    while (std::getline(file, line)) {
        if (line.compare(0, targetDate.size(), targetDate) == 0) {
            outputFile << line << '\n';
        } else {
            break;
        }
    }

    std::cout << "Log entries for " << targetDate << " saved to " << outputFileName << "\n";
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <log_file> <YYYY-MM-DD>\n";
        return 1;
    }

    std::string logFile = argv[1];
    std::string targetDate = argv[2];
    
    if (targetDate.size() != 10 || targetDate[4] != '-' || targetDate[7] != '-') {
        std::cerr << "Error: Date format should be YYYY-MM-DD\n";
        return 1;
    }

    filterLogsByDate(logFile, targetDate);
    return 0;
}
