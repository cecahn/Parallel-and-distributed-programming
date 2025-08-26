import numpy as np
import math
import os

def n_of_p(p, C):
    """Compute n(p) = C * sqrt(p) as an integer."""
    return int(C * math.sqrt(p))

# Process counts for weak scaling
process_counts = [1, 2, 4, 8, 16]

# Constants to test
constants = [10, 50, 100, 500, 1000, 1500]

# Create output directory
output_dir = "matrices"
os.makedirs(output_dir, exist_ok=True)

for C in constants:
    for p in process_counts:
        n = n_of_p(p, C)
        print(f"C = {C}, p = {p}: matrix size = {n} x {n}")

        # Create random matrix
        matrix = np.random.randint(0, 1_000_000_000, size=(n, n), dtype=np.int32)

        # Save to text file: first row is size, then matrix
        filename = os.path.join(output_dir, f"matrix_C{C}_p{p}.txt")
        with open(filename, "w") as f:
            f.write(f"{n}\n")  # write size of side
            np.savetxt(f, matrix, fmt="%d")



