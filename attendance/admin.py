from django.contrib import admin
from .models import Employee, Attendance, QRCode

@admin.register(Employee)
class EmployeeAdmin(admin.ModelAdmin):
    list_display = ('name', 'employee_id', 'department')
    search_fields = ('name', 'employee_id')
    list_filter = ('department',)

@admin.register(Attendance)
class AttendanceAdmin(admin.ModelAdmin):
    list_display = ('employee', 'timestamp', 'status', 'note')
    list_filter = ('status', 'timestamp', 'employee__department')
    search_fields = ('employee__name', 'employee__employee_id')
    date_hierarchy = 'timestamp'

@admin.register(QRCode)
class QRCodeAdmin(admin.ModelAdmin):
    list_display = ('employee', 'created_at', 'expires_at', 'is_used', 'is_valid')
    list_filter = ('is_used', 'created_at')
