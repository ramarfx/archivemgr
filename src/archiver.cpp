#include "archiver.hpp"
#include <fstream>
#include <cstring>
#include <algorithm>

namespace archiver {

    void printInfo(const std::string& msg) {
        std::cout << "[INFO] " << msg << "\n";
    }

    void printInfo(const FileEntry& entry) {
        std::cout << (entry.isDir ? "[DIR] " : "[FILE] ") 
                  << entry.name << " (" << entry.size << " bytes)\n";
    }

    void listZipContents(const std::string& archivePath, std::vector<FileEntry>& entries) {
        if (!fs::exists(archivePath)) {
            std::cerr << "Error: File not found at " << fs::absolute(archivePath) << "\n";
            return;
        }

        int err = 0;
        zip_t* archive = zip_open(archivePath.c_str(), 0, &err);
        
        if (!archive) {
            std::cerr << "Failed to open zip archive: " << archivePath << "\n";
            return;
        }

        zip_int64_t num_entries = zip_get_num_entries(archive, 0);
        for (zip_int64_t i = 0; i < num_entries; ++i) {
            zip_stat_t sb;
            if (zip_stat_index(archive, i, 0, &sb) == 0) {
                bool isDir = (sb.name[strlen(sb.name) - 1] == '/');
                entries.emplace_back(sb.name, sb.size, isDir);
            }
        }
        zip_close(archive);
    }

    bool isZipFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) return false;
        
        unsigned char magic[4];
        if (file.read(reinterpret_cast<char*>(magic), 4)) {
            return (magic[0] == 0x50 && magic[1] == 0x4B && magic[2] == 0x03 && magic[3] == 0x04);
        }
        return false;
    }

    void processExtractedFile(const std::string& outPath, const std::string& fileName) {
        if (isZipFile(outPath)) {
            std::string nestedOutputDir = outPath;
            if (fileName.length() >= 4 && fileName.substr(fileName.length() - 4) == ".zip") {
                nestedOutputDir = outPath.substr(0, outPath.length() - 4);
            } else {
                nestedOutputDir += "_extracted";
            }
            std::cout << "-> Found nested zip (by binary header): " << fileName 
                      << ", extracting recursively to " << nestedOutputDir << "\n";
            extractZip(outPath, nestedOutputDir);
        }
    }

    void extractZip(const std::string& archivePath, const std::string& outputDir) {
        if (!fs::exists(archivePath)) {
            std::cerr << "Error: File not found at " << fs::absolute(archivePath) << "\n";
            return;
        }

        int err = 0;
        zip_t* archive = zip_open(archivePath.c_str(), 0, &err);
        
        if (!archive) {
            std::cerr << "Failed to open zip archive for extraction: " << archivePath << "\n";
            return;
        }

        fs::create_directories(outputDir);
        zip_int64_t num_entries = zip_get_num_entries(archive, 0);

        for (zip_int64_t i = 0; i < num_entries; ++i) {
            zip_stat_t sb;
            if (zip_stat_index(archive, i, 0, &sb) == 0) {
                std::string outPath = outputDir + "/" + sb.name;
                
                if (sb.name[strlen(sb.name) - 1] == '/') {
                    fs::create_directories(outPath);
                } else {
                    fs::create_directories(fs::path(outPath).parent_path());
                    
                    zip_file_t* zf = zip_fopen_index(archive, i, 0);
                    if (!zf) continue;

                    std::ofstream outFile(outPath, std::ios::binary);
                    if (outFile.is_open()) {
                        char buffer[8192];
                        zip_int64_t bytesRead = 0;
                        while ((bytesRead = zip_fread(zf, buffer, sizeof(buffer))) > 0) {
                            outFile.write(buffer, bytesRead);
                        }
                        outFile.close();
                    }
                    zip_fclose(zf);
                    std::cout << "Extracted: " << sb.name << "\n";
                    
                    processExtractedFile(outPath, sb.name);
                }
            }
        }
        zip_close(archive);
    }

    void addPathToZip(zip_t* archive, const fs::path& p, const std::string& zipPath) {
        if (!fs::exists(p)) {
            std::cerr << "Path not found, skipping: " << p << "\n";
            return;
        }

        if (fs::is_directory(p)) {
            std::string dirName = zipPath;
            if (!dirName.empty() && dirName.back() != '/') {
                dirName += '/';
            }
            if (zip_dir_add(archive, dirName.c_str(), ZIP_FL_ENC_UTF_8) < 0) {
                // Ignore
            } else {
                std::cout << "Zipped DIR: " << p.string() << " -> " << dirName << "\n";
            }

            for (const auto& entry : fs::directory_iterator(p)) {
                addPathToZip(archive, entry.path(), dirName + entry.path().filename().string());
            }
        } else {
            zip_source_t* source = zip_source_file(archive, p.string().c_str(), 0, -1);
            if (!source) {
                std::cerr << "Failed to open source file: " << p << "\n";
                return;
            }
            if (zip_file_add(archive, zipPath.c_str(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
                std::cerr << "Failed to add file to zip: " << p << "\n";
                zip_source_free(source);
            } else {
                std::cout << "Zipped FILE: " << p.string() << " -> " << zipPath << "\n";
            }
        }
    }

    void createZip(const std::string& archivePath, const std::vector<std::string>& files) {
        int err = 0;
        zip_t* archive = zip_open(archivePath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &err);
        if (!archive) {
            std::cerr << "Failed to create zip archive: " << archivePath << "\n";
            return;
        }

        for (const auto& file : files) {
            fs::path p(file);
            addPathToZip(archive, p, p.filename().string());
        }
        zip_close(archive);
    }

    void recordHistory(const std::string& action) {
        static std::list<std::string> history;
        history.push_back(action);
        
        std::ofstream log("history.log", std::ios::app);
        if (log.is_open()) {
            log << action << "\n";
        }
    }

    void showHistory() {
        std::ifstream log("history.log");
        std::list<std::string> historyList;
        std::string line;
        while (std::getline(log, line)) {
            historyList.push_back(line);
        }
        
        printSeparator();
        std::cout << "Operation History (from log file):\n";
        printCollection(historyList);
        printSeparator();
    }

    void findInZip(const std::string& archivePath, const std::string& query) {
        std::vector<FileEntry> entries;
        listZipContents(archivePath, entries);

        // 1. Search/Find using std::find_if and Lambda
        auto it = std::find_if(entries.begin(), entries.end(), [&query](const FileEntry& e) {
            return e.name.find(query) != std::string::npos;
        });

        printSeparator();
        if (it != entries.end()) {
            std::cout << "Found matching entry:\n";
            printInfo(*it); // Overloaded printInfo
        } else {
            std::cout << "No exact match found for query: " << query << "\n";
        }

        // 2. Count matches using std::count_if and Lambda
        int matchCount = std::count_if(entries.begin(), entries.end(), [&query](const FileEntry& e) {
            return e.name.find(query) != std::string::npos;
        });
        std::cout << "Total matching entries: " << matchCount << "\n";
        printSeparator();
    }
}
