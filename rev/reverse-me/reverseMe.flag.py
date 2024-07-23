"""
Reverse Engineer this simple python program to get the flag
I bet this is an easy one for you, harper
"""

import base64
from cryptography.fernet import Fernet

key = b'4OS2iWWgPNIHduuqKVDTPZKYOMM88vKofMv27SJSgug='
payload = b'gAAAAABmiQIH_9Y129xpib48dZNrwOAzugg1uwAKvKjcfvFfuKgfJ8gWRrkd-C7v1zY0Cc4x7tCDys_w1YhIRSxSxZJJZTXVYbc2leE9PO9ZnSArWjglbYWUDBZqIPugDeYwIgn6HdGezLlbDm3sqrKlvw05ngkp8iunLYgOa36JJSWlKhzmVJajDOdcQTR_9M7bfQ5_IS4JNJfaJaP0b0mmVMiw4lwFGr1Ft5rJ4z16lbqsV-dCmWRfaQqupiBalnK7Y_bDhSXv'

f = Fernet(key)

plain = f.decrypt(payload)
exec(plain.decode())
