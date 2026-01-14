import uuid
from django.db import models
from django.utils import timezone
from datetime import timedelta

class Employee(models.Model):
    name = models.CharField(max_length=100)
    employee_id = models.CharField(max_length=20, unique=True)
    department = models.CharField(max_length=50)

    def __str__(self):
        return f"{self.name} ({self.employee_id})"

class QRCode(models.Model):
    token = models.UUIDField(default=uuid.uuid4, editable=False, unique=True)
    employee = models.ForeignKey(Employee, on_delete=models.CASCADE)
    created_at = models.DateTimeField(auto_now_add=True)
    expires_at = models.DateTimeField()
    is_used = models.BooleanField(default=False)

    def save(self, *args, **kwargs):
        if not self.expires_at:
            self.expires_at = timezone.now() + timedelta(minutes=1)
        super().save(*args, **kwargs)

    @property
    def is_valid(self):
        return (not self.is_used) and (timezone.now() <= self.expires_at)

class Attendance(models.Model):
    STATUS_CHOICES = [
        ('SUCCESS', 'Berhasil'),
        ('FAILED', 'Gagal'),
    ]
    employee = models.ForeignKey(Employee, on_delete=models.CASCADE, null=True, blank=True)
    timestamp = models.DateTimeField(auto_now_add=True)
    status = models.CharField(max_length=10, choices=STATUS_CHOICES)
    qr_code = models.ForeignKey(QRCode, on_delete=models.SET_NULL, null=True, blank=True)
    note = models.CharField(max_length=255, blank=True, null=True)

    def __str__(self):
        return f"{self.employee} - {self.timestamp} - {self.status}"
