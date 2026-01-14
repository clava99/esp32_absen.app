# Akses Admin Panel (Jazzmin)

Aplikasi ini menggunakan **Jazzmin** untuk mempercantik interface Django Admin.

## URL Akses

Silakan akses URL berikut di browser Anda:

```
http://localhost:8000/admin/
```

*Sesuaikan `localhost:8000` dengan domain/IP server jika sudah di-deploy.*

> **Catatan:**
> Karena sistem memiliki halaman login custom, Anda akan diarahkan ke halaman login aplikasi terlebih dahulu.
> **Setelah login berhasil, Anda akan otomatis diarahkan kembali ke Jazzmin.**

## Kredensial Login

Gunakan akun Superadmin yang telah dibuat.

- **Username**: `admin` (default, atau sesuai yang Anda buat)
- **Password**: (sesuai yang Anda set)

> **Lupa Password?**
> Lihat panduan [Reset Password Superadmin](reset_password_superadmin.md) untuk mereset password Anda.

## Fitur Dashboard Jazzmin

Setelah login, Anda akan melihat dashboard admin dengan antarmuka Jazzmin yang lebih modern, yang mencakup:

- **Menu Navigasi Sidebar**: Akses cepat ke model-model aplikasi.
- **Top Menu**: Link cepat ke Dashboard Monitoring (`/dashboard/`).
- **Pencarian Global**: Mencari data Employee atau User dengan cepat.
- **Tema Custom**: Menggunakan tema "flatly" dengan sidebar gelap (configurable di `settings.py`).
