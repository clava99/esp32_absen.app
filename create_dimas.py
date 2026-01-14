import os
import django

os.environ.setdefault("DJANGO_SETTINGS_MODULE", "core.settings")
django.setup()

from attendance.models import Employee

def run():
    employee_id = '101'
    name = 'Dimas'
    department = 'General'

    if not Employee.objects.filter(employee_id=employee_id).exists():
        Employee.objects.create(name=name, employee_id=employee_id, department=department)
        print(f"Employee '{name}' with ID '{employee_id}' created successfully.")
    else:
        print(f"Employee with ID '{employee_id}' already exists.")

if __name__ == '__main__':
    run()
