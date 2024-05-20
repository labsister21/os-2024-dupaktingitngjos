# IF2230 - 2024 - dupaktingtingjOS
> *Source Code* ini dibuat oleh Kelompok dupaktingtingjOS, untuk memenuhi Tugas Besar Sistem Operasi yaitu membuat sistem operasi yang berjalan pada arsitektur x86 32-bit

## Daftar Isi
- [Deskripsi Singkat](#deskrips-singkat)
- [Sistematika Folder](#sistematika-folder)
- [Requirements](#requirements)
- [Cara Mengkompilasi dan Menjalankan Program](#cara-mengkompilasi-dan-menjalankan-program)
- [Author](#author)

## Deskripsi Singkat
Selamat datang di repository Tugas Besar IF2230 - 2024 dupaktingtingjOS! Tugas besar ini akan membuat sebuah sistem operasi yang akan mengenalkan kepada pengembangan kernel dan akan mempelajari secara konkrit subsistem yang ada pada sistem operasi. Target platform untuk sistem operasi ini adalah x86 32-bit Protected Mode, dan kernel yang dikembangkan akan dijalankan menggunakan QEMU.

### Milestone 0 - Pembuatan Sistem Operasi x86: Toolchain, Kernel, GDT
Waktu implementasi : Senin, 26 Februari 2024 - Sabtu, 9 Maret 2024
1. Menyiapkan *Repository* dan *Tools*
2. Kernel Dasar
3. Otomatisasi Build
4. Menjalankan Sistem Operasi
5. Pembuatan Struktur data GDT
6. Load GDT

### Milestone 1 - Pembuatan Sistem Operasi x86: Interrupt, Driver, dan File System
Waktu implementasi : Minggu, 10 Maret 2024 - Sabtu, 6 April 2024
1. *Text Framebuffer*
2. *Interrupt*
3. *Keyboard Driver*
4. *File System*

### Milestone 2 - Pembuatan Sistem Operasi x86: Paging, User Mode, dan Shell
Waktu implementasi : Minggu, 7 April 2024 - Sabtu, 27 April 2024
1. Manajemen *Memory*
2. Separasi Kernel-User *Space*
3. *Shell*

## Sistematika Folder
```bash
.
├─── .github
├─── .vscode
├─── bin
├─── other
├─── src
│     ├─── ch0
│     │     ├─── gdt
│     │     ├─── kernel-entrypoint
│     │     └─── stdlib
│     ├─── ch1
│     │     ├─── disk
│     │     ├─── fat32
│     │     ├─── framebuffer
│     │     ├─── idt
│     │     ├─── interrupt
│     │     ├─── intsetup
│     │     ├─── keyboard
│     │     └─── portio
│     └─── ch2
│           ├─── crt0
│           ├─── external-inserter
│           ├─── paging
│           └─── user-shell
├─── makefile
└─── README.md
```

## Requirements
- GCC compiler (versi 11.2.0 atau yang lebih baru)
- Visual Studio Code
- Windows Subsystem for Linux (WSL2) dengan distribusi minimal Ubuntu 20.04
- Emulator QEMU

## Cara Mengkompilasi dan Menjalankan Program
1. Lakukan *clone repository* melalui terminal dengan *command* berikut
    ``` bash
    $ git clone https://github.com/labsister21/os-2024-dupaktingitngjos.git
    ```
2. Lakukan eksekusi pada makefile dengan memasukkan *command* `make` pada terminal. 
3. Jika berhasil maka akan tercipta beberapa file pada folder `bin` dan sistem operasi akan muncul pada layar.
4. Jika proses aktivasi tidak berhasil, maka gunakan [Panduan Debugger dan WSL](https://docs.google.com/document/d/1Zt3yzP_OEiFz8g2lHlpBNNr9qUyXghFNeQlAeQpAaII/edit#heading=h.z69qt6rveqcu).

## Author
| NIM      | Nama                       |
| -------- | ---------------------------|
| 13522006 | Agil Fadillah Sabri        |
| 13522034 | Bastian H Suryapratama     |
| 13522056 | Diero Arga Purnama         |
| 13522076 | Muhammad Syarafi Akmal     |