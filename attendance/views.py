import json
import base64
import qrcode
from io import BytesIO
from django.shortcuts import render, get_object_or_404, redirect
from django.utils import timezone
from django.http import JsonResponse
from rest_framework.decorators import api_view
from rest_framework.response import Response
from rest_framework import status
from django.db import transaction
from .models import Employee, QRCode, Attendance

# --- Views for Web Frontend ---

# def index(request):
#     if request.session.get('employee_id'):
#         return redirect('employee_dashboard')
#     return redirect('employee_login')

def employee_login(request):
    if request.method == 'POST':
        emp_id = request.POST.get('employee_id')
        try:
            employee = Employee.objects.get(employee_id=emp_id)
            request.session['employee_id'] = employee.id
            return redirect('employee_dashboard')
        except Employee.DoesNotExist:
            return render(request, 'login_employee.html', {'error': 'ID Karyawan tidak ditemukan'})
    
    return render(request, 'login_employee.html')

def employee_logout(request):
    request.session.flush()
    return redirect('employee_login')

def employee_dashboard(request):
    emp_id = request.session.get('employee_id')
    if not emp_id:
        return redirect('employee_login')
    
    employee = get_object_or_404(Employee, id=emp_id)
    attendance_list = Attendance.objects.filter(employee=employee).order_by('-timestamp')
    
    # Simple Stats
    now = timezone.now()
    stats = {
        'total': attendance_list.count(),
        'this_month': attendance_list.filter(timestamp__month=now.month, timestamp__year=now.year).count(),
        'last_seen': attendance_list.first().timestamp if attendance_list.exists() else None
    }
    
    return render(request, 'dashboard_employee.html', {
        'employee': employee,
        'attendance_list': attendance_list,
        'stats': stats
    })

# --- Admin Views ---

def admin_login_view(request):
    next_url = request.GET.get('next')
    if request.user.is_authenticated and request.user.is_staff:
        if next_url:
            return redirect(next_url)
        return redirect('admin_dashboard')
        
    if request.method == 'POST':
        from django.contrib.auth import authenticate, login
        u = request.POST.get('username')
        p = request.POST.get('password')
        next_post = request.POST.get('next')

        user = authenticate(request, username=u, password=p)
        if user is not None and user.is_staff:
            login(request, user)
            if next_post:
                return redirect(next_post)
            return redirect('admin_dashboard')
        else:
            return render(request, 'login_admin.html', {'error': 'Username atau password salah.', 'next': next_post})

    return render(request, 'login_admin.html', {'next': next_url})

def admin_login_views(request):
    next_url = request.GET.get('next')
    if request.user.is_authenticated and request.user.is_staff:
        if next_url:
            return redirect(next_url)
        return redirect('admin_dashboard')
        
    if request.method == 'POST':
        from django.contrib.auth import authenticate, login
        u = request.POST.get('username')
        p = request.POST.get('password')
        next_post = request.POST.get('next')

        user = authenticate(request, username=u, password=p)
        if user is not None and user.is_staff:
            login(request, user)
            if next_post:
                return redirect(next_post)
            return redirect('admin_dashboard')
        else:
            return render(request, 'login_admin.html', {'error': 'Username atau password salah.', 'next': next_post})

    return render(request, 'login_admin.html')

def admin_logout_view(request):
    from django.contrib.auth import logout
    logout(request)
    return redirect('employee_login')

from django.db.models import Min, Max, Count
from django.utils import timezone

def admin_dashboard_view(request):
    if not request.user.is_staff:
        return redirect('admin_login')
    
    today = timezone.now().date()
    
    # 1. Get all employees
    employees = Employee.objects.all()
    
    # 2. Prepare dashboard data
    employee_data = []
    present_today = 0
    
    for emp in employees:
        # Get Check In (Earliest SUCCESS scan today)
        check_in = Attendance.objects.filter(
            employee=emp, 
            status='SUCCESS', 
            timestamp__date=today
        ).aggregate(Min('timestamp'))['timestamp__min']
        
        # Get Check Out (Latest SUCCESS scan today)
        check_out = Attendance.objects.filter(
            employee=emp, 
            status='SUCCESS', 
            timestamp__date=today
        ).aggregate(Max('timestamp'))['timestamp__max']
        
        if check_in:
            present_today += 1
            
        employee_data.append({
            'employee': emp,
            'check_in': check_in,
            'check_out': check_out
        })
    
    # Quick Stats
    total_employees = employees.count()
    total_scans_today = Attendance.objects.filter(timestamp__date=today, status='SUCCESS').count()
    
    stats = {
        'total_employees': total_employees,
        'present_today': present_today,
        'absent_today': total_employees - present_today,
        'total_scans': total_scans_today
    }
    
    return render(request, 'dashboard_admin.html', {
        'employee_data': employee_data, 
        'stats': stats, 
        'today': today
    })

def add_employee_view(request):
    if not request.user.is_staff:
        return redirect('admin_login')
        
    if request.method == 'POST':
        name = request.POST.get('name')
        emp_id = request.POST.get('employee_id')
        dept = request.POST.get('department')
        
        if Employee.objects.filter(employee_id=emp_id).exists():
             return render(request, 'add_employee.html', {'error': 'ID Karyawan sudah digunakan!'})
             
        Employee.objects.create(name=name, employee_id=emp_id, department=dept)
        return redirect('admin_dashboard')
        
    return render(request, 'add_employee.html')

def admin_employee_detail_view(request, employee_id):
    if not request.user.is_staff:
        return redirect('admin_login')
        
    employee = get_object_or_404(Employee, id=employee_id)
    attendance_list = Attendance.objects.filter(employee=employee).order_by('-timestamp')
    
    # Calculate simple stats
    total_present = attendance_list.filter(status='SUCCESS').count()
    
    return render(request, 'admin_employee_detail.html', {
        'employee': employee,
        'attendance_list': attendance_list,
        'total_present': total_present
    })

def dashboard(request):
     # Deprecated legacy dashboard, redirect to new one
     return redirect('admin_dashboard')

def generate_qr_page(request):
    emp_id = request.session.get('employee_id')
    
    # If not logged in, force login
    if not emp_id:
        return redirect('employee_login')

    employee = get_object_or_404(Employee, id=emp_id)
    context = {'user_employee': employee}
    
    if request.method == "POST":
        # Create new QR Code for current user
        qr_obj = QRCode.objects.create(employee=employee)
        
        # Generate QR Image
        qr_img = qrcode.make(str(qr_obj.token))
        buffer = BytesIO()
        qr_img.save(buffer, format="PNG")
        img_str = base64.b64encode(buffer.getvalue()).decode()
        
        context['qr_image'] = img_str
        context['qr_token'] = qr_obj.token
        context['expires_at'] = qr_obj.expires_at
        
    return render(request, 'generate_qr.html', context)

# --- REST API for ESP32 ---

import logging

logger = logging.getLogger(__name__)

@api_view(['POST'])
def validate_scan(request):
    """
    Endpoint for ESP32-CAM.
    Expected JSON: { "qr_token": "..." }
    """
    token = request.data.get('qr_token')
    
    if not token:
        logger.warning("validate_scan: No token provided")
        return Response({'status': 'FAIL', 'message': 'No token provided'}, status=status.HTTP_400_BAD_REQUEST)

    try:
        # Use a transaction and lock the QR row to avoid race conditions
        with transaction.atomic():
            # Select for update to lock the row
            qr_obj = QRCode.objects.select_for_update().get(token=token)
            employee = qr_obj.employee

            if qr_obj.is_used:
                logger.info(f"validate_scan: Token {token} already used by {employee}")
                # Record failed attempt (Duplicate)
                Attendance.objects.create(
                    employee=employee,
                    qr_code=qr_obj,
                    status='FAILED',
                    note='QR Code already used'
                )
                return Response({'status': 'FAIL', 'message': 'QR Code already used'}, status=status.HTTP_400_BAD_REQUEST)

            if timezone.now() > qr_obj.expires_at:
                logger.info(f"validate_scan: Token {token} expired for {employee}")
                # Record failed attempt (Expired)
                Attendance.objects.create(
                    employee=employee,
                    qr_code=qr_obj,
                    status='FAILED',
                    note='QR Code expired'
                )
                return Response({'status': 'FAIL', 'message': 'QR Code expired'}, status=status.HTTP_400_BAD_REQUEST)

            # Success flow: mark used and save attendance
            qr_obj.is_used = True
            qr_obj.save(update_fields=['is_used']) # Explicitly update only this field

            Attendance.objects.create(
                employee=employee,
                qr_code=qr_obj,
                status='SUCCESS',
                note='Attendance recorded'
            )
            
            logger.info(f"validate_scan: Success for {employee} with token {token}")

            return Response({
                'status': 'SUCCESS',
                'message': f'Welcome, {employee.name}',
                'employee': employee.name,
                'time': timezone.now().strftime('%Y-%m-%d %H:%M:%S')
            })

    except QRCode.DoesNotExist:
        logger.warning(f"validate_scan: Invalid token {token}")
        return Response({'status': 'FAIL', 'message': 'Invalid QR Token'}, status=status.HTTP_404_NOT_FOUND)
    except Exception as e:
        logger.error(f"validate_scan: Error {str(e)}", exc_info=True)
        return Response({'status': 'ERROR', 'message': str(e)}, status=status.HTTP_500_INTERNAL_SERVER_ERROR)
