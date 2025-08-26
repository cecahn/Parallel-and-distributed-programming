#!/bin/bash
#SBATCH -A uppmax2025-3-7
#SBATCH -M snowy
#SBATCH -p node
#SBATCH --ntasks-per-node=16
#SBATCH --job-name=shear-sort
#SBATCH -t 00:30:00
#SBATCH --output=job.%j.out
#SBATCH --error=job.%j.err

module load gcc
module load openmpi

echo "Running 10 load per process"
echo "1 processes"
mpirun -n 1 ./shearsort matrices/matrix_C10_p1.txt output.txt

echo "2 processes"
mpirun -n 2 ./shearsort matrices/matrix_C10_p2.txt output.txt

echo "4 processes"
mpirun -n 4 ./shearsort matrices/matrix_C10_p4.txt output.txt

echo "8 processes"
mpirun -n 8 ./shearsort matrices/matrix_C10_p8.txt output.txt

echo "16 processes"
mpirun -n 16 ./shearsort matrices/matrix_C10_p16.txt output.txt

echo "Running 50 load per process"
echo "1 processes"
mpirun -n 1 ./shearsort matrices/matrix_C50_p1.txt output.txt

echo "2 processes"
mpirun -n 2 ./shearsort matrices/matrix_C50_p2.txt output.txt

echo "4 processes"
mpirun -n 4 ./shearsort matrices/matrix_C50_p4.txt output.txt

echo "8 processes"
mpirun -n  8./shearsort matrices/matrix_C50_p8.txt output.txt

echo "16 processes"
mpirun -n 16 ./shearsort matrices/matrix_C50_p16.txt output.txt


echo "Running 100 load per process"
echo "1 processes"
mpirun -n 1 ./shearsort matrices/matrix_C100_p1.txt output.txt

echo "2 processes"
mpirun -n 2 ./shearsort matrices/matrix_C100_p2.txt output.txt

echo "4 processes"
mpirun -n 4 ./shearsort matrices/matrix_C100_p4.txt output.txt

echo "8 processes"
mpirun -n 8 ./shearsort matrices/matrix_C100_p8.txt output.txt

echo "16 processes"
mpirun -n 16 ./shearsort matrices/matrix_C100_p16.txt output.txt

echo "Running 500 load per process"
echo "1 processes"
mpirun -n 1 ./shearsort matrices/matrix_C500_p1.txt output.txt

echo "2 processes"
mpirun -n 2 ./shearsort matrices/matrix_C500_p2.txt output.txt

echo "4 processes"
mpirun -n 4 ./shearsort matrices/matrix_C500_p4.txt output.txt

echo "8 processes"
mpirun -n 8 ./shearsort matrices/matrix_C500_p8.txt output.txt

echo "16 processes"
mpirun -n 16 ./shearsort matrices/matrix_C500_p16.txt output.txt

echo "Running 1000 load per process"
echo "1 processes"
mpirun -n 1 ./shearsort matrices/matrix_C1000_p1.txt output.txt

echo "2 processes"
mpirun -n 2 ./shearsort matrices/matrix_C1000_p2.txt output.txt

echo "4 processes"
mpirun -n 4 ./shearsort matrices/matrix_C1000_p4.txt output.txt

echo "8 processes"
mpirun -n 8 ./shearsort matrices/matrix_C1000_p8.txt output.txt

echo "16 processes"
mpirun -n 16 ./shearsort matrices/matrix_C1000_p16.txt output.txt

echo "Running 1500 load per process"

echo "1 process"
mpirun -n 1 ./shearsort matrices/matrix_C1500_p1.txt output.txt

echo "2 processes"
mpirun -n 2 ./shearsort matrices/matrix_C1500_p2.txt output.txt

echo "4 processes"
mpirun -n 4 ./shearsort matrices/matrix_C1500_p4.txt output.txt

echo "8 processes"
mpirun -n 8 ./shearsort matrices/matrix_C1500_p8.txt output.txt

echo "16 processes"
mpirun -n 16 ./shearsort matrices/matrix_C1500_p16.txt output.txt