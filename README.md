```markdown
# Sistem Manajemen Perpustakaan Berbasis CLI

Project UAS Lab Pemrograman Kelas A — Informatika FMIPA Universitas Syiah Kuala 2026.

Aplikasi manajemen perpustakaan berbasis Command Line Interface (CLI) yang ditulis dalam C11. Mendukung dua role (admin dan user), pencarian buku, pengurutan katalog, serta peminjaman dan pengembalian buku.

## Anggota Kelompok

| No | Nama Lengkap          | NPM              |
|----|-----------------------|------------------|
| 1  | Shykara Akbari YS     | 250810701100018  |
| 2  | Asyifa                | 260810701100016  |
| 3  | Nabila Muksitha       | 260810701100024  |

## Fitur

- Login (3x kesempatan) + daftar akun baru (otomatis role `user`)
- Admin: tambah, edit, hapus buku; cari buku; lihat katalog; laporan seluruh peminjaman
- User: lihat katalog (hanya buku yang tersedia); cari buku; pinjam; kembalikan; lihat peminjaman sendiri
- Katalog bisa diurut berdasarkan ID, judul, tahun, jumlah halaman, atau stok (naik/turun) memakai Bubble Sort, Selection Sort, atau Insertion Sort
- Hasil katalog dan pencarian dipaginasi 20 buku per halaman

## Arsitektur

Semua data dimuat ke memori dalam satu struct `Library`, diubah di situ, lalu disimpan kembali ke file.

Alur program (`src/main.c`):
1. Load data buku, akun, dan peminjaman dari folder `data/`
2. Login atau daftar akun baru
3. Masuk menu sesuai role (`adminMenu` atau `userMenu`)
4. Saat logout, simpan perubahan ke file lalu free memori

Struktur data (`include/types.h`):
- `Book`: ocaid, judul, penulis, tahun, halaman, edisi, work, URL, stok
- `Account`: username, password, role
- `Loan`: id, username, ocaid, tanggal, status (`DIPINJAM` / `DIKEMBALIKAN`)
- `Library`: pembungkus tiga array dinamis (buku, akun, peminjaman) + count dan capacity. Array membesar otomatis dengan `realloc`

Pengurutan (`src/sort.c`):
Tiga algoritma (Bubble, Selection, Insertion) ditulis manual. Hanya mengurutkan array indeks supaya struct `Book` tidak perlu dipindah-pindah di memori.

Penyimpanan:
Kalau `data/inventory.txt` belum ada, program membuatnya dari `data/Books.txt` dengan stok awal 3. Setelah itu aplikasi hanya baca/tulis `inventory.txt`. `Books.txt` tetap jadi dataset asli.

## Struktur File

```text
uas-perpustakaan-final/
├── Makefile
├── README.md
├── data/
│   ├── Books.txt         # dataset asli
│   ├── inventory.txt     # katalog + stok yang dipakai
│   ├── user.txt          # username password role
│   └── peminjaman.txt    # riwayat transaksi
├── include/
│   ├── types.h
│   ├── auth.h
│   ├── book.h
│   ├── fileio.h
│   ├── loan.h
│   ├── sort.h
│   └── utils.h
├── src/
│   ├── main.c
│   ├── auth.c
│   ├── book.c
│   ├── loan.c
│   ├── sort.c
│   ├── fileio.c
│   └── utils.c
└── tests/
    └── sorttest.c
```

## Cara Menjalankan

Wajib dijalankan dari root repo (folder yang berisi `Makefile`, `src`, `include`, dan `data`). Kalau tidak, file data tidak ketemu.

```bash
git clone https://github.com/shykaraakbari09-coder/uas-perpustakaan-final.git
cd uas-perpustakaan-final
```

### Linux
```bash
# Arch / EndeavourOS
sudo pacman -S base-devel

# Ubuntu / Debian
sudo apt install build-essential

# Fedora
sudo dnf install make gcc

make run
```

### macOS
```bash
xcode-select --install
make run
```

### Windows

Paling gampang pakai WSL:
```powershell
wsl --install
# restart, buka Ubuntu, lalu:
sudo apt install build-essential
make run
```

Tanpa WSL (MSYS2):
1. Install MSYS2 dari https://www.msys2.org
2. Buka UCRT64 shell, jalankan `pacman -S mingw-w64-ucrt-x86_64-gcc`
3. Tambah `C:\msys64\ucrt64\bin` ke PATH
4. Dari root repo di PowerShell:
```powershell
mkdir build
gcc -std=c11 -Wall -Wextra -pedantic -Iinclude -o build/library.exe src/*.c
.\build\library.exe
```

### Compile manual (tanpa make)
```bash
mkdir -p build
gcc -std=c11 -Wall -Wextra -pedantic -Iinclude -o build/library src/*.c
./build/library
```

### Target Makefile
| Perintah     | Fungsi                     |
|--------------|----------------------------|
| `make`       | Compile ke `build/library` |
| `make run`   | Compile lalu jalankan      |
| `make clean` | Hapus folder `build`       |

## Akun Contoh

| Username | Password | Role  |
|----------|----------|-------|
| admin    | admin123 | admin |
| budi     | budi123  | user  |
| sari     | sari123  | user  |
```

---

## 🪟 Alternatif Windows (Tanpa WSL/MSYS2)
Bagi pengguna Windows yang hanya memiliki compiler standar (seperti Dev-C++, Code::Blocks, atau Visual Studio), kami menyediakan file `windows_main.c` yang menggabungkan seluruh modul menjadi satu file.

1. Buka `windows_main.c` di IDE C Anda.
2. Pastikan folder `data/` berada di direktori yang sama dengan executable.
3. Compile dan jalankan. (Jika menggunakan Command Prompt: `gcc -std=c11 windows_main.c -o library.exe && library.exe`)
