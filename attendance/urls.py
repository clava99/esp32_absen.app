from django.urls import path
from . import views

urlpatterns = [
    # path('', views.index, name='index'),
    # Employee Routes
    path('', views.employee_login, name='employee_login'),
    path('logout/', views.employee_logout, name='employee_logout'),
    path('my-dashboard/', views.employee_dashboard, name='employee_dashboard'),
    path('generate/', views.generate_qr_page, name='generate_qr'),
    
    # Admin Routes
    # path('admin/login/', views.admin_login_view, name='admin_login'),
    path('admin/logout/', views.admin_logout_view, name='admin_logout'),
    path('dashboard/', views.admin_dashboard_view, name='admin_dashboard'),
    path('admin/employee/<int:employee_id>/', views.admin_employee_detail_view, name='admin_employee_detail'),
    path('admin/add-employee/', views.add_employee_view, name='add_employee'),
    
    # API
    path('api/scan/', views.validate_scan, name='api_scan'),
    path('api/attendance/', views.validate_scan, name='api_scan_alias'),
]
