from django.contrib.auth.models import User
from attendance.models import Employee

def run():
    """This script creates a superuser and a sample employee."""

    # --- Create Superuser ---
    admin_username = 'admin'
    admin_password = 'admin'
    if not User.objects.filter(username=admin_username).exists():
        User.objects.create_superuser(admin_username, 'admin@example.com', admin_password)
        print(f"Superuser '{admin_username}' created with password '{admin_password}'.")
    else:
        print(f"Superuser '{admin_username}' already exists.")
        # Optionally, update password if you want to ensure it's known
        # user = User.objects.get(username=admin_username)
        # user.set_password(admin_password)
        # user.save()

    # --- Create Employee ---
    employee_id = '12345'
    if not Employee.objects.filter(employee_id=employee_id).exists():
        Employee.objects.create(name='Gusti Permana', employee_id=employee_id, department='Development')
        print(f"Employee 'Gusti Permana' with ID '{employee_id}' created.")
    else:
        print(f"Employee with ID '{employee_id}' already exists.")

    print("\nSetup script finished.")
    print("You can now run 'python manage.py runserver' and log in.")

# This allows the script to be run with 'python manage.py shell < create_data.py'
run()
