# Recursive Archive Manager

Recursive Archive Manager adalah aplikasi CLI sederhana yang menggunakan library eksternal `libzip` untuk melakukan operasi manipulasi `.zip`. Aplikasi ini memiliki fitur unik yaitu dapat melakukan operasi ekstraksi secara rekursif dan mendeteksi file zip di dalam zip meskipun disamarkan dengan ekstensi palsu. Sehingga cocok digunakan sebagai tools CTF.

## Fitur Unggulan Terkini
1. **Modular Architecture**: Struktur kode dipisah secara modular antara Header (`archiver.hpp`), Source Implementation (`archiver.cpp`), dan Entry Point (`main.cpp`).
2. **Recursive Extraction & Binary Header Detection**: Ekstraksi mampu mendeteksi file zip di dalam zip (*nested zip*) meskipun disamarkan dengan ekstensi palsu (seperti `.pdf` atau `.png`) menggunakan pembacaan *binary magic number* `PK\x03\x04`, lalu mengekstraknya secara rekursif.
3. **Directory Zipping**: Mendukung kompresi sebuah folder beserta seluruh isi file dan sub-foldernya secara otomatis.
4. **Flexible Output (`-o`)**: Opsi kustomisasi output letak direktori/file baik saat zipping (`create`) maupun unzipping (`extract`).

## Konsep C++ yang Digunakan (Kriteria UAS)
Aplikasi ini mendemonstrasikan:
1. **Namespace**: Mengelompokkan fungsionalitas di dalam `namespace archiver`.
2. **Struct & Default Argument**: Menggunakan `struct FileEntry` dengan argumen default pada konstruktor.
3. **Pointers & References**: Penggunaan pointer pada API C `libzip` (`zip_t*`, `zip_file_t*`) dan referensi saat mem-passing parameter.
4. **Function Overloading & Templates**: Template fungsi `printCollection` dan overloading fungsi `printInfo`.
5. **STL Containers**: Menggunakan `std::vector` untuk daftar file dan `std::list` untuk mencatat riwayat.
6. **Iterators**: Loop menggunakan iterator untuk menjelajahi elemen STL.
7. **Algorithms**: Penggunaan `std::sort`, `std::count_if`, dan `std::max_element`.
8. **File Handling**: Menggunakan `std::ifstream` / `std::ofstream` untuk ekstrak file, membaca binary header, dan menulis log riwayat.
9. **Lambda Expressions**: Digunakan dalam fungsi algoritma untuk kondisi kustom.

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
```

**2. Melihat isi file ZIP (`list`)**
```bash
./archivemgr list arsip_saya.zip
```

**3. Mengekstrak file ZIP (`extract`)**
```bash
# Mengekstrak ke current directory
./archivemgr extract arsip_saya.zip

# Mengekstrak ke folder tujuan khusus menggunakan flag -o
./archivemgr extract arsip_saya.zip -o folder_tujuan
```

*(Catatan: Setiap aktivitas akan dicatat otomatis ke dalam file `history.log` sebagai demonstrasi fungsionalitas File Handling).*
