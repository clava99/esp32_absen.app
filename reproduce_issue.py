
import os
import django
import json
from django.test import RequestFactory
from django.utils import timezone
import datetime

os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'core.settings')
django.setup()

from attendance.models import Employee, QRCode, Attendance
from attendance.views import validate_scan

def run_repro():
    print("--- Setting up Reproduction ---")
    # Clean up
    Attendance.objects.all().delete()
    QRCode.objects.all().delete()
    Employee.objects.all().delete()

    # Create Employee
    emp = Employee.objects.create(name='TestUser', employee_id='T999', department='Dev')
    print(f"Created Employee: {emp}")

    # Create QR
    qr = QRCode.objects.create(employee=emp)
    print(f"Created QR: {qr.token} (is_used={qr.is_used})")

    # Simulate Request
    factory = RequestFactory()
    data = {'qr_token': str(qr.token)}
    request = factory.post('/api/scan/', data=data, content_type='application/json')
    
    # Execute View
    print("\n--- Executing validate_scan ---")
    response = validate_scan(request)
    print(f"Response Status: {response.status_code}")
    print(f"Response Data: {response.data}")

    # Verify State
    print("\n--- Verifying State ---")
    qr.refresh_from_db()
    print(f"QR is_used: {qr.is_used}")
    
    try:
        att = Attendance.objects.get(qr_code=qr)
        print(f"Attendance Status: {att.status}")
    except Attendance.DoesNotExist:
        print("Attendance: NOT FOUND")

    if response.status_code == 200 and qr.is_used and att.status == 'SUCCESS':
        print("\nRESULT: Backend works correctly.")
    else:
        print("\nRESULT: FAILURE detected.")

if __name__ == '__main__':
    run_repro()
