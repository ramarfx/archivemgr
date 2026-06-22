#ifndef ARCHIVER_HPP
#define ARCHIVER_HPP

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <zip.h>

namespace fs = std::filesystem;

namespace archiver {

struct FileEntry {
  std::string name;
  size_t size;
  bool isDir;
  bool isEncrypted;

  FileEntry(std::string n = "", size_t s = 0, bool d = false, bool enc = false)
      : name(n), size(s), isDir(d), isEncrypted(enc) {}
};

inline void printSeparator() {
  std::cout << "========================================\n";
}

template <typename Container> void printCollection(const Container &c) {
  for (auto it = c.begin(); it != c.end(); ++it) {
    std::cout << "- " << *it << "\n";
  }
}

void printInfo(const std::string &msg);
void printInfo(const FileEntry &entry);

void listZipContents(const std::string &archivePath,
                     std::vector<FileEntry> &entries);

bool isZipFile(const std::string &filePath);
void processExtractedFile(const std::string &outPath,
                          const std::string &fileName,
                          const std::string &password = "");
void extractZip(const std::string &archivePath, const std::string &outputDir,
                const std::string &password = "");

void addPathToZip(zip_t *archive, const fs::path &p,
                  const std::string &zipPath, const std::string &password = "");
void createZip(const std::string &archivePath,
               const std::vector<std::string> &files,
               const std::string &password = "");

void recordHistory(const std::string &action);
void showHistory();
void findInZip(const std::string &archivePath, const std::string &query);

} // namespace archiver

#endif // ARCHIVER_HPP
