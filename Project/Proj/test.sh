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

for procs in 1 2 4 8 16
do
    echo "Running 5x5 with $procs processes"
    mpirun -n $procs shearsort 5x5.txt output_5x5_p${procs}.txt
done

for procs in 1 2 4 8 16
do
    echo "Running 10x10 with $procs processes"
    mpirun -n $procs shearsort 10x10.txt output_10x10_p${procs}.txt
done

for procs in 1 2 4 8 16
do
    echo "Running 20x20 with $procs processes"
    mpirun -n $procs shearsort 20x20.txt output_20x20_p${procs}.txt
done