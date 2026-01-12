# PANDUAN PENGGUNAAN SISTEM ABSENSI QR CODE

## A. Pendahuluan

Sistem ini dirancang untuk memudahkan pencatatan kehadiran karyawan menggunakan teknologi QR Code dinamis (berubah setiap menit) yang dipindai oleh perangkat IoT (ESP32-CAM) atau divalidasi oleh sistem backend. Sistem memisahkan hak akses antara **Karyawan** (Absensi) dan **Administrator** (Monitoring & Manajemen).

---

## B. Fitur Utama

1.  **QR Code Dinamis**: Kode QR valid hanya selama 60 detik untuk mencegah kecurangan (titip absen).
2.  **Sistem Login Terpisah**: Portal khusus Karyawan dan Administrator.
3.  **Real-time Dashboard**:
    - **Karyawan**: Melihat riwayat diri sendiri.
    - **Admin**: Melihat "Jam Masuk" dan "Jam Keluar" seluruh karyawan hari ini.
4.  **Detail Riwayat**: Admin dapat melihat rekap lengkap per karyawan.
5.  **Integrasi IoT**: API siap pakai untuk hardware ESP32-CAM.

---

## C. Alur Kerja Sistem (Flowchart)

### 1. Alur Absensi (Karyawan)

1.  **Login**: Karyawan masuk ke web menggunakan `ID Karyawan`.
2.  **Generate QR**: Menekan tombol "Buat QR Absen".
3.  **Scan**: Menghadapkan layar ke alat scanner (ESP32).
4.  **Validasi**:
    - Jika < 60 detik & belum dipakai -> **Sukses**.
    - Jika > 60 detik / sudah dipakai -> **Gagal**.

### 2. Alur Manajemen (Admin)

1.  **Login**: Masuk sebagai Admin.
2.  **Monitoring**: Melihat tabel absensi hari ini.
    - _Scan Pertama_ = Check In.
    - _Scan Terakhir_ = Check Out.
3.  **Detail**: Klik nama karyawan untuk melihat rekap bulanan/tahunan.
4.  **Register**: Menambah data karyawan baru.

---

## D. Cara Pengoperasian

### BAGIAN 1: ADMINISTRATOR

#### 1. Login Admin

- Buka URL: `/admin/login/` (atau klik "Admin Login" di pojok kanan atas).
- Masukkan Username & Password Admin.

#### 2. Menambah Karyawan Baru

- Di Dashboard, klik tombol **"Tambah Karyawan"**.
- Isi:
  - **Nama Lengkap**: (Misal: Budi Santoso)
  - **ID Karyawan**: (Harus unik, Misal: IT001)
  - **Departemen**: Pilih dari daftar.
- Klik **Simpan**.

#### 3. Monitoring Harian (Dashboard Utama)

- Buka menu **Dashboard**.
- Lihat kotak statistik (Total Karyawan, Hadir, dll).
- Pada tabel, kolom **Jam Masuk** menunjukkan waktu datang, dan **Jam Keluar** otomatis terupdate jika karyawan melakukan scan terakhir sebelum pulang.

#### 4. Melihat Riwayat Lengkap Karyawan

- Di tabel Dashboard, **klik nama karyawan** yang ingin dicek.
- Sistem akan membuka halaman **Detail Karyawan**.
- Anda dapat melihat seluruh riwayat "Berhasil" atau "Gagal" dari karyawan tersebut beserta jam detailnya.

---

### BAGIAN 2: KARYAWAN

#### 1. Melakukan Absensi

- Buka halaman utama: `/`.
- Klik **Login Karyawan**.
- Masukkan ID Karyawan Anda.
- Di Dashboard Pribadi, klik tombol **"Buat QR Absen"**.
- Tunjukkan QR Code yang muncul ke kamera scanner.
- _Tunggu bunyi/indikator sukses pada alat scanner._

#### 2. Mengecek Riwayat Sendiri

- Setelah login, di halaman Dashboard Pribadi, gulir ke bawah.
- Tabel **Riwayat Kehadiran** menampilkan kapan saja Anda berhasil absen.

---

## E. Troubleshooting (Masalah Umum)

1.  **QR Code Expired (Kadaluarsa)**
    - _Penyebab_: QR Code sudah lebih dari 60 detik ditampilkan.
    - _Solusi_: Klik "Generate Ulang" atau refresh halaman.
2.  **Gagal Scan (Duplicate)**
    - _Penyebab_: QR Code yang sama discan dua kali.
    - _Solusi_: Generate QR Code baru.
3.  **Data Tidak Muncul di Dashboard**
    - _Solusi_: Refresh halaman. Dashboard Admin menghitung data secara real-time.

---

_Dokumen ini dibuat otomatis sebagai bagian dari pengembangan sistem._
