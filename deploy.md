# Panduan Build & Deploy (Recursive Archive Manager)

Panduan ini menjelaskan cara melakukan kompilasi program `archivemgr` dalam dua mode: **Build Biasa (Test Mode)** dan **Build Deploy (Deployment Mode)**, serta cara melakukan deploy ulang (redeploy) jika Anda melakukan perubahan kode.

---

## 1. Persiapan Awal (Hanya Sekali)
Pastikan Anda berada di direktori project utama:
```bash
cd archivemgr-simple
```

Jika belum ada folder `build`, buat terlebih dahulu:
```bash
mkdir -p build
```

---

## 2. Build Biasa (Test Mode)
Mode ini digunakan untuk pengujian lokal di komputer Anda sendiri. Program akan mencari library `libzip` yang terinstall di OS sistem.

**Cara Build:**
```bash
cd build
cmake -DDEPLOY_BUILD=OFF ..
make
```
*(Catatan: Anda juga bisa mengetik `cmake ..` saja karena `DEPLOY_BUILD` secara default bernilai `OFF`)*

---

## 3. Build Deploy (Deployment Mode)
Mode ini digunakan saat Anda siap membagikan aplikasi ke komputer Linux lain tanpa mengharuskan mereka menginstal library `libzip` terlebih dahulu. 

Pada mode ini, CMake akan mendownload source-code `libzip` langsung dari internet dan membangunnya secara statis (`libzip.a`) untuk kemudian ditanamkan langsung ke dalam satu file executable `archivemgr`.

**Cara Build:**
```bash
cd build
cmake -DDEPLOY_BUILD=ON ..
make
```

**Hasil Build Deploy:**
Di dalam folder `build/`, Anda hanya akan melihat **satu file executable tunggal** bernama `archivemgr` tanpa ada file `.so` tambahan yang menemaninya.

**Cara Distribusi/Share:**
Kirimkan satu file `archivemgr` ini saja. Teman atau dosen Anda bisa langsung meletakkan file tersebut di mana saja di komputer Linux mereka dan langsung menjalankannya secara portabel.

---

## 4. Cara Deploy Ulang (Redeploy)
Jika Anda melakukan perubahan pada kode C++ (`src/main.cpp` atau `src/archiver.cpp`) dan ingin membuat binary deploy yang baru, ikuti langkah-langkah pembersihan dan build ulang berikut agar konfigurasinya bersih.

### Langkah Cepat (Clean & Rebuild):
```bash
cd build
# Hapus cache cmake lama (disarankan agar tidak bentrok)
rm -rf CMakeCache.txt CMakeFiles/

# Lakukan build ulang dengan mode deploy
cmake -DDEPLOY_BUILD=ON ..
make
```

Dengan perintah di atas, binary `archivemgr` yang baru akan dikompilasi ulang secara statis menggunakan kode terbaru Anda.

---

## 5. Cara Build untuk Windows (.exe)
Jika Anda ingin menghasilkan aplikasi berbentuk `.exe` agar bisa dijalankan secara portabel di Windows, Anda bisa melakukan kompilasi silang (*cross-compile*) langsung dari terminal WSL/Linux.

### Persiapan:
Pastikan compiler Windows MinGW sudah terinstal di WSL Anda:
```bash
sudo apt update
sudo apt install g++-mingw-w64-x86-64
```

### Langkah Kompilasi untuk Windows:
1. Buat folder build khusus Windows (misal `build_win`):
   ```bash
   mkdir -p build_win
   cd build_win
   ```
2. Jalankan CMake dengan konfigurasi toolchain target Windows dan mode deploy:
   ```bash
   cmake -DCMAKE_SYSTEM_NAME=Windows \
         -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
         -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
         -DDEPLOY_BUILD=ON ..
   ```
3. Lakukan proses build:
   ```bash
   make
   ```

Setelah proses selesai, file **`archivemgr.exe`** akan muncul di dalam folder `build_win/`. File ini bersifat mandiri (single executable) dan bisa langsung dicopy lalu dijalankan di Windows mana saja tanpa perlu menginstal apa pun lagi.
