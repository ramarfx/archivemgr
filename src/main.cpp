#include "archiver.hpp"
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

void printUsage() {
    std::cout << "Simple Archive Manager CLI\n";
    std::cout << "Usage:\n";
    std::cout << "  archivemgr create [-o <archive.zip>] <file1> <file2> ...\n";
    std::cout << "  archivemgr list <archive.zip>\n";
    std::cout << "  archivemgr extract <archive.zip> [-o <output_dir>]\n";
}

int main(int argc, char* argv[]) {
    using namespace archiver;

    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "list") {
        if (argc < 3) {
            std::cerr << "Archive file required for listing.\n";
            printUsage();
            return 1;
        }
        std::string archivePath = argv[2];
        std::vector<FileEntry> entries;
        listZipContents(archivePath, entries);

        printInfo("Archive contents:");

        std::sort(entries.begin(), entries.end(), [](const FileEntry& a, const FileEntry& b) {
            return a.name < b.name;
        });

        int fileCount = std::count_if(entries.begin(), entries.end(), [](const FileEntry& e) {
            return !e.isDir;
        });

        for (const auto& entry : entries) {
            printInfo(entry);
        }

        std::cout << "\nTotal files: " << fileCount << "\n";
        
        auto largest = std::max_element(entries.begin(), entries.end(), [](const FileEntry& a, const FileEntry& b) {
            return a.size < b.size;
        });

        if (largest != entries.end() && !largest->isDir) {
            std::cout << "Largest file: " << largest->name << " (" << largest->size << " bytes)\n";
        }

        recordHistory("Listed contents of " + archivePath);

    } else if (command == "extract") {
        if (argc < 3) {
            std::cerr << "Archive file required for extraction.\n";
            printUsage();
            return 1;
        }
        std::string archivePath = argv[2];
        std::string outputDir = "."; // Default output dir

        for (int i = 3; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-o") {
                if (i + 1 < argc) {
                    outputDir = argv[i + 1];
                    i++;
                } else {
                    std::cerr << "Error: -o flag requires a directory path.\n";
                    return 1;
                }
            } else {
                outputDir = arg; // Support legacy positional argument
            }
        }

        extractZip(archivePath, outputDir);
        recordHistory("Extracted " + archivePath + " to " + outputDir);
        
    } else if (command == "create") {
        std::string archivePath = "archive.zip"; // Default output zip
        std::vector<std::string> files;

        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-o") {
                if (i + 1 < argc) {
                    archivePath = argv[i + 1];
                    i++; // skip the path argument
                } else {
                    std::cerr << "Error: -o flag requires a file path.\n";
                    return 1;
                }
            } else {
                files.push_back(arg);
            }
        }

        if (files.empty()) {
            std::cerr << "At least one file is required to create a zip.\n";
            printUsage();
            return 1;
        }

        createZip(archivePath, files);
        recordHistory("Created archive " + archivePath + " with " + std::to_string(files.size()) + " files");

    } else {
        std::cerr << "Unknown command: " << command << "\n";
        printUsage();
        return 1;
    }

    return 0;
}
