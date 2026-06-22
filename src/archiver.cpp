#include "archiver.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <list>

using namespace std;

namespace archiver {

void printInfo(const string &msg) { cout << "[INFO] " << msg << "\n"; }

void printInfo(const FileEntry &entry) {
  cout << (entry.isDir ? "[DIR] " : "[FILE] ") << entry.name;
  if (entry.isEncrypted) {
    cout << " (encrypted)";
  }
  cout << " (" << entry.size << " bytes)\n";
}

void listZipContents(const string &archivePath, vector<FileEntry> &entries) {
  if (!fs::exists(archivePath)) {
    cerr << "Error: File not found at " << fs::absolute(archivePath) << "\n";
    return;
  }

  int err = 0;
  zip_t *archive = zip_open(archivePath.c_str(), 0, &err);

  if (!archive) {
    cerr << "Failed to open zip archive: " << archivePath << "\n";
    return;
  }

  zip_int64_t num_entries = zip_get_num_entries(archive, 0);

  for (zip_int64_t i = 0; i < num_entries; ++i) {
    zip_stat_t sb;
    if (zip_stat_index(archive, i, 0, &sb) == 0) {
      bool isDir = (sb.name[strlen(sb.name) - 1] == '/');
      bool isEncrypted = false;
      if (sb.valid & ZIP_STAT_ENCRYPTION_METHOD) {
        isEncrypted = (sb.encryption_method != ZIP_EM_NONE);
      }
      entries.emplace_back(sb.name, sb.size, isDir, isEncrypted);
    }
  }
  zip_close(archive);
}

bool isZipFile(const string &filePath) {
  ifstream file(filePath, ios::binary);
  if (!file.is_open())
    return false;

  unsigned char magic[4];
  if (file.read(reinterpret_cast<char *>(magic), 4)) {
    return (magic[0] == 0x50 && magic[1] == 0x4B && magic[2] == 0x03 &&
            magic[3] == 0x04);
  }
  return false;
}

void processExtractedFile(const string &outPath, const string &fileName, const string &password) {
  if (isZipFile(outPath)) {
    string nestedOutputDir = outPath;
    if (fileName.length() >= 4 &&
        fileName.substr(fileName.length() - 4) == ".zip") {
      nestedOutputDir = outPath.substr(0, outPath.length() - 4);
    } else {
      nestedOutputDir += "_extracted";
    }
    cout << "-> Found nested zip (by binary header): " << fileName
         << ", extracting recursively to " << nestedOutputDir << "\n";
    extractZip(outPath, nestedOutputDir, password);
  }
}

void extractZip(const string &archivePath, const string &outputDir, const string &password) {
  if (!fs::exists(archivePath)) {
    cerr << "Error: File not found at " << fs::absolute(archivePath) << "\n";
    return;
  }

  int err = 0;
  zip_t *archive = zip_open(archivePath.c_str(), 0, &err);

  if (!archive) {
    cerr << "Failed to open zip archive for extraction: " << archivePath
         << "\n";
    return;
  }

  if (!password.empty()) {
    if (zip_set_default_password(archive, password.c_str()) < 0) {
      cerr << "Warning: Failed to set password for extraction.\n";
    }
  }

  fs::create_directories(outputDir);
  zip_int64_t num_entries = zip_get_num_entries(archive, 0);

  for (zip_int64_t i = 0; i < num_entries; ++i) {
    zip_stat_t sb;
    if (zip_stat_index(archive, i, 0, &sb) == 0) {
      string outPath = outputDir + "/" + sb.name;

      if (sb.name[strlen(sb.name) - 1] == '/') {
        fs::create_directories(outPath);
      } else {
        fs::create_directories(fs::path(outPath).parent_path());

        zip_file_t *zf = zip_fopen_index(archive, i, 0);
        if (!zf) {
          cerr << "Error: Failed to open file inside zip: " << sb.name;
          if (password.empty() && (sb.valid & ZIP_STAT_ENCRYPTION_METHOD) && sb.encryption_method != ZIP_EM_NONE) {
            cerr << " (this file is encrypted, password required. Use -p)";
          } else if (!password.empty()) {
            cerr << " (wrong password?)";
          }
          cerr << "\n";
          continue;
        }

        ofstream outFile(outPath, ios::binary);
        if (outFile.is_open()) {
          char buffer[8192];
          zip_int64_t bytesRead = 0;
          while ((bytesRead = zip_fread(zf, buffer, sizeof(buffer))) > 0) {
            outFile.write(buffer, bytesRead);
          }
          outFile.close();
        }
        zip_fclose(zf);
        cout << "Extracted: " << sb.name << "\n";

        processExtractedFile(outPath, sb.name, password);
      }
    }
  }
  zip_close(archive);
}

void addPathToZip(zip_t *archive, const fs::path &p, const string &zipPath, const string &password) {
  if (!fs::exists(p)) {
    cerr << "Path not found, skipping: " << p << "\n";
    return;
  }

  if (fs::is_directory(p)) {
    string dirName = zipPath;
    if (!dirName.empty() && dirName.back() != '/') {
      dirName += '/';
    }
    if (zip_dir_add(archive, dirName.c_str(), ZIP_FL_ENC_UTF_8) < 0) {
      // Ignore
    } else {
      cout << "Zipped DIR: " << p.string() << " -> " << dirName << "\n";
    }

    for (const auto &entry : fs::directory_iterator(p)) {
      addPathToZip(archive, entry.path(),
                   dirName + entry.path().filename().string(), password);
    }
  } else {
    zip_source_t *source = zip_source_file(archive, p.string().c_str(), 0, -1);
    if (!source) {
      cerr << "Failed to open source file: " << p << "\n";
      return;
    }
    zip_int64_t idx = zip_file_add(archive, zipPath.c_str(), source,
                                   ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
    if (idx < 0) {
      cerr << "Failed to add file to zip: " << p << "\n";
      zip_source_free(source);
    } else {
      if (!password.empty()) {
        if (zip_file_set_encryption(archive, idx, ZIP_EM_TRAD_PKWARE, password.c_str()) < 0) {
          cerr << "Warning: Failed to encrypt file " << p.string() << " (error: " << zip_strerror(archive) << ")\n";
        }
      }
      cout << "Zipped FILE: " << p.string() << " -> " << zipPath << (password.empty() ? "" : " (encrypted)") << "\n";
    }
  }
}

void createZip(const string &archivePath, const vector<string> &files, const string &password) {
  int err = 0;
  zip_t *archive =
      zip_open(archivePath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &err);
  if (!archive) {
    cerr << "Failed to create zip archive: " << archivePath << "\n";
    return;
  }

  for (const auto &file : files) {
    fs::path p(file);
    addPathToZip(archive, p, p.filename().string(), password);
  }
  zip_close(archive);
}

void recordHistory(const string &action) {
  static list<string> history;
  history.push_back(action);

  ofstream log("history.log", ios::app);
  if (log.is_open()) {
    log << action << "\n";
  }
}

void showHistory() {
  ifstream log("history.log");
  list<string> historyList;
  string line;
  while (getline(log, line)) {
    historyList.push_back(line);
  }

  printSeparator();
  cout << "Operation History (from log file):\n";
  printCollection(historyList);
  printSeparator();
}

void findInZip(const string &archivePath, const string &query) {
  vector<FileEntry> entries;
  listZipContents(archivePath, entries);

  // 1. Search/Find using find_if and Lambda
  auto it =
      find_if(entries.begin(), entries.end(), [&query](const FileEntry &e) {
        return e.name.find(query) != string::npos;
      });

  printSeparator();
  if (it != entries.end()) {
    cout << "Found matching entry:\n";
    printInfo(*it); // Overloaded printInfo
  } else {
    cout << "No exact match found for query: " << query << "\n";
  }

  // 2. Count matches using count_if and Lambda
  int matchCount =
      count_if(entries.begin(), entries.end(), [&query](const FileEntry &e) {
        return e.name.find(query) != string::npos;
      });
  cout << "Total matching entries: " << matchCount << "\n";
  printSeparator();
}
} // namespace archiver
