# Dokumentasi Sistem Absensi QR Code (IoT + Django)

## 1. Deskripsi Sistem

Sistem ini adalah aplikasi absensi berbasis web yang menggunakan QR Code dinamis dan ESP32-CAM sebagai pemindai.
Arsitektur sistem:

- **Backend**: Django (Python) + Django REST Framework.
- **Frontend**: Django Templates + **Tailwind CSS**.
- **Scanner**: ESP32-CAM (IoT Device).

## 2. Cara Menjalankan Web Absensi

### Prasyarat

- Python 3.x
- `pip`

### Langkah-langkah

1. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```
2. Migrasi database:
   ```bash
   python manage.py migrate
   ```
3. Buat Superuser (untuk Admin Panel):
   ```bash
   python manage.py createsuperuser
   ```
4. Jalankan server:
   ```bash
   python manage.py runserver 0.0.0.0:8000
   ```

## 3. Fitur & Alur Kerja

### A. Sisi Karyawan

1. **Login**: `http://localhost:8000/login/` (Gunakan ID Karyawan).
2. **Dashboard Pribadi**: Melihat statistik kehadiran sendiri.
3. **Generate QR**: Membuat QR Code untuk absen (berlaku 60 detik).

### B. Sisi Admin (Pengurus)

1. **Login Admin**: `http://localhost:8000/admin/login/` (Akun Superuser).
2. **Dashboard Admin**:
   - Melihat ringkasan kehadiran hari ini (Total Karyawan, Hadir, Belum Hadir).
   - **Tabel Monitoring**: Menampilkan jam **Masuk** (Check In) dan jam **Keluar** (Check Out) per karyawan hari ini.
3. **Tambah Karyawan**:
   - Menu khusus untuk mendaftarkan karyawan baru beserta Departemennya.

## 4. Integrasi ESP32-CAM (API)

- **Endpoint**: `/api/scan/` (POST JSON)
- **Logika**:
  - Scan pertama hari ini = **Check In**
  - Scan berikutnya = Update status kehadiran / Catatan Scan
  - Sistem Dashboard otomatis mendeteksi scan paling awal sebagai "Masuk" dan paling akhir sebagai "Keluar".

## 5. Struktur Database Utama

- **Employee**: Nama, ID, Departemen.
- **Attendance**: Log waktu scan.
- **QRCode**: Token validasi sementara.

Untuk mengakses sistem ini dari HP atau perangkat lain yang berada dalam jaringan Wi-Fi yang sama, ikuti langkah berikut:

Pastikan HP dan Laptop terhubung ke Wi-Fi yang sama.
Buka browser di HP (Chrome/Safari).
Ketik alamat berikut: http://192.168.100.9:8000
Maka tampilan web akan muncul di HP Anda.

Catatan:

Jika alamat di atas tidak bisa diakses, kemungkinan ada Firewall di laptop yang memblokir. (Namun biasanya fitur bawaan Django runserver ini cukup untuk demo skripsi).