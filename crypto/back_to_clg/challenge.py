from Crypto.Cipher import AES
from Crypto.Util.Padding import pad
import hashlib
import sympy as sp
import random

flag = b'REDACTED'

key = random.getrandbits(7)

x = sp.symbols('x')

f = "REDACTED"
f_first = "REDACTED"
f_second = "REDACTED"

assert(2*f_second + 1*f_first - 6*f == 0)
assert(f.subs(x, 0) | f_first.subs(x, 0) == 18)

def encrypt(message, key):
    global f
    global x
    expand = f.subs(x, key).evalf(100)
    expand_hash = hashlib.sha256(str(expand).encode()).digest()[:16]
    cipher = AES.new(expand_hash, AES.MODE_CFB)
    iv = cipher.iv
    ciphertext = cipher.encrypt(pad(message, AES.block_size))
    return iv.hex() + ciphertext.hex()

encrypted = encrypt(flag, key)

print(key)
print(encrypted)