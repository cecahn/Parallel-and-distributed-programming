import matplotlib.pyplot as plt

# Raw timing data
processes = [1, 2, 4, 8, 16]
matrix_sizes = ['10x10', '50x50', '100x100', '500x500', '1000x1000', '1500x1500']

# Execution times for each matrix size (indexed by process count)
execution_times = [
    # Running 10 load per process
    [0.000125, 0.000257, 0.000363, 0.000693, 0.001501],  # 1, 2, 4, 8, 16 processes

    # Running 50 load per process
    [0.001907, 0.002509, 0.002431, 0.003928, 0.003928],  # 1, 2, 4, 8, 16 processes 

    # Running 100 load per process
    [0.009264, 0.009184, 0.009006, 0.010041, 0.011596],  # 1, 2, 4, 8, 16 processes

    # Running 500 load per process
    [0.224959, 0.248659, 0.268330, 0.274818, 0.279339],  # 1, 2, 4, 8, 16 processes

    # Running 1000 load per process
    [1.012862, 1.135166, 1.011557, 1.290749, 1.265560],  # 1, 2, 4, 8, 16 processes

    # Running 1500 load per process
    [2.371744, 2.663335, 2.843158, 3.277362, 3.090183],  # 1, 2, 4, 8, 16 processes
]

# Compute speedups: speedup = time with 1 process / time with n processes
speedups = []
for times in execution_times:
    base_time = times[0]
    speedups.append([base_time / t for t in times])

# Plotting
plt.figure(figsize=(10, 6))

for i, size in enumerate(matrix_sizes):
    plt.plot(processes, speedups[i], marker='o', label=size)

# Formatting
plt.title('Speedup vs Number of Processes')
plt.xlabel('Number of Processes')
plt.ylabel('Speedup (T1 / Tn)')
plt.xticks(processes)
plt.grid(True, linestyle='--', alpha=0.6)
plt.legend(title='Matrix Size')
plt.tight_layout()

# Show plot
plt.show()


