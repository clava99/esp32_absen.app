from django.contrib.auth import logout
from django.shortcuts import redirect

def custom_logout(request):
    """
    Log out the user and redirect to the admin login page.
    This view supports GET requests, fixing the 405 error in Django 5+.
    """
    logout(request)
    return redirect('/admin/')
