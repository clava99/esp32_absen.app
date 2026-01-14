# Reset Password Superadmin

Dokumentasi ini menjelaskan cara mereset password superadmin untuk aplikasi Django ini.

## Mengetahui Username Superadmin

Berdasarkan default setup (`setup_users.py`), username default untuk superadmin adalah **`admin`**.

## Metode 1: Menggunakan Command `changepassword` (Paling Mudah)

Jika Anda memiliki akses terminal ke server/environment, ini adalah cara termudah.

1.  Pastikan `genv` aktif.
2.  Jalankan perintah berikut:

    ```bash
    python manage.py changepassword admin
    ```

    *Ganti `admin` dengan username superuser Anda jika berbeda.*

3.  Masukkan password baru dua kali saat diminta.

## Metode 2: Menggunakan Django Shell

Jika metode pertama tidak berhasil atau Anda perlu cara alternatif:

1.  Masuk ke Django shell:

    ```bash
    python manage.py shell
    ```

2.  Jalankan script python berikut:

    ```python
    from django.contrib.auth.models import User
    
    # Ganti 'admin' dengan username target
    u = User.objects.get(username='admin')
    
    # Set password baru
    u.set_password('password_baru_anda')
    
    # Simpan
    u.save()
    ```

3.  Keluar dari shell dengan `exit()` atau Ctrl+D.
