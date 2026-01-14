from django.test import TestCase
from django.urls import reverse
from django.utils import timezone
from datetime import timedelta
from .models import Employee, QRCode, Attendance
import uuid
import json

class ValidateScanTests(TestCase):
    def setUp(self):
        self.emp = Employee.objects.create(name='Test User', employee_id='T001', department='IT')

    def post_token(self, token):
        url = reverse('api_scan')
        return self.client.post(url, data=json.dumps({'qr_token': str(token)}), content_type='application/json')

    def test_validate_scan_success(self):
        qr = QRCode.objects.create(employee=self.emp)
        resp = self.post_token(qr.token)
        self.assertEqual(resp.status_code, 200)
        self.assertEqual(resp.json().get('status'), 'SUCCESS')
        self.assertTrue(Attendance.objects.filter(qr_code=qr, status='SUCCESS').exists())

    def test_validate_scan_reused(self):
        qr = QRCode.objects.create(employee=self.emp)
        self.post_token(qr.token)
        resp = self.post_token(qr.token)
        self.assertEqual(resp.status_code, 400)
        self.assertEqual(resp.json().get('status'), 'FAIL')
        self.assertTrue(Attendance.objects.filter(qr_code=qr, status='FAILED', note__icontains='already used').exists())

    def test_validate_scan_expired(self):
        expired_at = timezone.now() - timedelta(minutes=5)
        qr = QRCode.objects.create(employee=self.emp, expires_at=expired_at)
        resp = self.post_token(qr.token)
        self.assertEqual(resp.status_code, 400)
        self.assertEqual(resp.json().get('status'), 'FAIL')
        self.assertTrue(Attendance.objects.filter(qr_code=qr, status='FAILED', note__icontains='expired').exists())

    def test_validate_scan_invalid(self):
        resp = self.post_token(uuid.uuid4())
        self.assertEqual(resp.status_code, 404)
        self.assertEqual(resp.json().get('status'), 'FAIL')