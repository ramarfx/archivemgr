# Recursive Archive Manager

Recursive Archive Manager adalah aplikasi CLI sederhana yang menggunakan library eksternal `libzip` untuk melakukan operasi manipulasi `.zip`. Aplikasi ini memiliki fitur unik yaitu dapat melakukan operasi ekstraksi secara rekursif dan mendeteksi file zip di dalam zip meskipun disamarkan dengan ekstensi palsu. Sehingga cocok digunakan sebagai tools CTF.

## Fitur Unggulan Terkini
1. **Modular Architecture**: Struktur kode dipisah secara modular antara Header (`archiver.hpp`), Source Implementation (`archiver.cpp`), dan Entry Point (`main.cpp`).
2. **Recursive Extraction & Binary Header Detection**: Ekstraksi mampu mendeteksi file zip di dalam zip (*nested zip*) meskipun disamarkan dengan ekstensi palsu (seperti `.pdf` atau `.png`) menggunakan pembacaan *binary magic number* `PK\x03\x04`, lalu mengekstraknya secara rekursif.
3. **Directory Zipping**: Mendukung kompresi sebuah folder beserta seluruh isi file dan sub-foldernya secara otomatis.
4. **Flexible Output (`-o`)**: Opsi kustomisasi output letak direktori/file baik saat zipping (`create`) maupun unzipping (`extract`).
5. **Password Protection & Encryption (`-p`)**: Mendukung pembuatan arsip terenkripsi (metode Traditional PKWARE) dan ekstraksi arsip ber-password menggunakan flag `-p`. File terenkripsi akan diberi tanda `(encrypted)` pada daftar isi ZIP.

## Konsep C++ yang Digunakan (Kriteria UAS) & Pemetaan Kode

Berikut adalah daftar penerapan konsep pemrograman C++ lanjutan yang diimplementasikan pada project ini beserta lokasinya di dalam kode:

| No | Konsep C++ | File Lokasi | Baris Kode | Deskripsi Penerapan |
|----|------------|-------------|------------|---------------------|
| 1 | **STRUCT** | `src/archiver.hpp` | [L15-L22](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.hpp#L15-L22) | Menggunakan `struct FileEntry` untuk menampung metadata file (nama, ukuran, tipe direktori) yang ada di dalam arsip ZIP. |
| 2 | **REFERENCES & POINTERS** | `src/archiver.cpp` <br> `src/archiver.hpp` | [L16](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.cpp#L16), [L23](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.cpp#L23), [L93](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.cpp#L93) | - **Pointer**: `zip_t* archive` dan `zip_file_t* zf` untuk mengakses API libzip.<br> - **Reference**: Pass-by-reference `const std::string& archivePath` dan `std::vector<FileEntry>& entries` untuk efisiensi memori. |
| 3 | **NAMESPACE** | `src/archiver.hpp` <br> `src/archiver.cpp` | [L13](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.hpp#L13), [L5](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.cpp#L5) | Dibungkus di dalam `namespace archiver` untuk mencegah tabrakan nama fungsi (name collision) dan menjaga kerapian modularitas. |
| 4 | **Function** | `src/archiver.cpp` | [L16](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.cpp#L16), [L66](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.cpp#L66) | Seluruh logika utama dipisah menjadi fungsi-fungsi modular seperti `listZipContents`, `extractZip`, `createZip`, dll. |
| 5 | **DEFAULT ARGUMENT & INLINE FUNCTION** | `src/archiver.hpp` | [L20](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.hpp#L20), [L24-L26](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.hpp#L24-L26) | - **Default Argument**: `FileEntry(std::string n = "", ...)` memberikan nilai default jika struct diinisialisasi kosong.<br> - **Inline Function**: `inline void printSeparator()` digunakan untuk mencetak garis pembatas secara cepat tanpa overhead pemanggilan fungsi. |
| 6 | **FUNCTION OVERLOADING & FUNCTION TEMPLATE** | `src/archiver.hpp` <br> `src/archiver.cpp` | [L35-L36](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.hpp#L35-L36), [L7-L14](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.cpp#L7-L14), [L28-L33](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.hpp#L28-L33) | - **Function Overloading**: Fungsi `printInfo` di-overload untuk mencetak string pesan biasa ATAU mencetak struct `FileEntry`. <br> - **Function Template**: `printCollection` menerima tipe container apa saja (`std::vector` atau `std::list`) untuk dicetak isinya. |
| 7 | **STL (Standard Template Library): Vector dan List** | `src/main.cpp` <br> `src/archiver.cpp` | [L32](file:///home/ramarfx/projects/archivemgr-simple/src/main.cpp#L32), [L173](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.cpp#L173) | - **Vector**: `std::vector<FileEntry> entries` digunakan karena membutuhkan alokasi memori berurutan yang dinamis untuk file. <br> - **List**: `std::list<std::string> history` digunakan untuk mencatat riwayat karena efisiensi operasi insertion di akhir list. |
| 8 | **ITERATOR** | `src/archiver.hpp` | [L30](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.hpp#L30) | Menggunakan iterator `auto it = c.begin(); it != c.end(); ++it` pada function template `printCollection` untuk menjelajahi elemen container secara generik. |
| 9 | **Sort, find, dan count** | `src/main.cpp` <br> `src/archiver.cpp` | [L37](file:///home/ramarfx/projects/archivemgr-simple/src/main.cpp#L37), [L192](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.cpp#L192), [L41](file:///home/ramarfx/projects/archivemgr-simple/src/main.cpp#L41) | - **Sort**: `std::sort` mengurutkan file berdasarkan nama secara alfabetis.<br> - **Find**: `std::find_if` mencari file tertentu di dalam ZIP berdasarkan kueri pencarian.<br> - **Count**: `std::count_if` menghitung total file (tidak termasuk direktori) di dalam ZIP. |
| 10 | **File Handling** | `src/archiver.cpp` | [L42](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.cpp#L42), [L96](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.cpp#L96), [L175](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.cpp#L175) | - **ifstream**: Membaca binary header ZIP (`PK\x03\x04`) dan membaca log history.<br> - **ofstream**: Menulis data file hasil ekstraksi dan mencatat riwayat operasi ke `history.log`. |
| 11 | **Lambda Expressions** | `src/main.cpp` <br> `src/archiver.cpp` | [L37](file:///home/ramarfx/projects/archivemgr-simple/src/main.cpp#L37), [L41](file:///home/ramarfx/projects/archivemgr-simple/src/main.cpp#L41), [L192](file:///home/ramarfx/projects/archivemgr-simple/src/archiver.cpp#L192) | Menggunakan lambda `[](const FileEntry& a, const FileEntry& b) { ... }` untuk kriteria pengurutan, penghitungan, dan pencarian elemen secara *inline* tanpa mendefinisikan fungsi eksternal. |

## Requirements
Untuk kompilasi dan menjalankan program ini, pastikan sudah menginstal library pendukung (`libzip` dan `pkg-config`):

```bash
sudo apt update
sudo apt install libzip-dev cmake build-essential pkg-config
```

## Compile Project
Gunakan CMake untuk *compile* project ini:

```bash
cd archivemgr-simple
mkdir build
cd build
cmake ..
make
```

## Cara Menggunakan (CLI)

Setelah dikompilasi, Anda bisa menjalankan perintah-perintah berikut:

**1. Membuat Arsip ZIP (`create`)**
```bash
# Membuat arsip ke file default (archive.zip)
./archivemgr create file1.txt file2.txt folder_anda

# Membuat arsip dengan nama khusus menggunakan flag -o
./archivemgr create -o arsip_saya.zip file1.txt file2.txt folder_anda

# Membuat arsip dengan proteksi password menggunakan flag -p
./archivemgr create -o arsip_terkunci.zip -p rahasia123 file1.txt
```

**2. Melihat isi file ZIP (`list`)**
```bash
# File terenkripsi akan secara otomatis ditandai dengan label (encrypted)
./archivemgr list arsip_terkunci.zip
```

**3. Mengekstrak file ZIP (`extract`)**
```bash
# Mengekstrak ke current directory
./archivemgr extract arsip_saya.zip

# Mengekstrak ke folder tujuan khusus menggunakan flag -o
./archivemgr extract arsip_saya.zip -o folder_tujuan

# Mengekstrak file ZIP ber-password menggunakan flag -p
./archivemgr extract arsip_terkunci.zip -o folder_tujuan -p rahasia123
```

*(Catatan: Setiap aktivitas akan dicatat otomatis ke dalam file `history.log` sebagai demonstrasi fungsionalitas File Handling).*
