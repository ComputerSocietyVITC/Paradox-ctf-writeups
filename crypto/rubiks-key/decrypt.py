# Generates the private keys. Equation: x = (public^private_a) % public2 and vice versa
def generate_key(public1, public2, private_a, private_b):
    x = pow(public1, private_a) % public2
    y = pow(public1, private_b) % public2
    return (x, y)


# Generates the symmetric key. Equation: x = (public^y) % public2
def generate_sym_key(public1, public2, gen_key):
    x = pow(public1, gen_key) % public2
    return x


def decrypt():
    # Y Perm = FRU'R'U'RUR'F'RUR'U'R'FRF' (without spaces)
    public1 = 7082853982398539828582397039828582398539823970827039  # Y Perm converted to ascii values and combined
    public2 = 100000  # aka modulo
    private_a = 2
    private_b = 3
    x, y = generate_key(public1, public2, private_a, private_b)
    print(x, y)

    # Recursively exchanging and generating symmetric keys for 90 times
    for i in range(ord("Z")):
        print(i)
        x, y = generate_sym_key(public1, public2, y), generate_sym_key(
            public1, public2, x
        )

    # Final key
    return (x, y)


a, b = decrypt()
print(a, b)
