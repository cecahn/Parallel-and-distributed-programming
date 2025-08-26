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
    echo "Running 100x100 with $procs processes"
    mpirun -n $procs ./a.out 100m.txt output_100x100_p${procs}.txt
done

for procs in 1 2 4 8 16
do
    echo "Running 500x500 with $procs processes"
    mpirun -n $procs ./a.out 500m.txt output_500x500_p${procs}.txt
done

for procs in 1 2 4 8 16
do
    echo "Running 1000x1000 with $procs processes"
    mpirun -n $procs ./a.out 1000m.txt output_1000x1000_p${procs}.txt
done

for procs in 1 2 4 8 16
do
    echo "Running 10000x10000 with $procs processes"
    mpirun -n $procs ./a.out 10000m.txt output_10000x10000_p${procs}.txt
done

for procs in 1 2 4 8 16
do
    echo "Running 5000x5000 with $procs processes"
    mpirun -n $procs ./a.out 5000m.txt output_5000x5000_p${procs}.txt
done


