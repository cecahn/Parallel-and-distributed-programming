Shear sort
This program uses the Shear sort algorithm to sort numbers in a texfile in a snake like order. 
These are displayed in a matrix of nxn size where even numbered rows are sorted in ascending 
order adn uneven rows are sorted in descending order. 

How to run the program
In order to run the program one must prepare an environment where openmpi is installed
To compile the program you start by running make which creates the executable shearsort
This executable requires two arguments an "in" textfile containing a NxN matrix and an "out" textfile
The executable i runned using Mpirun specifying the number of processes with the flag -n 

Example
mpirun -n 4 ./shearsort 5x5.txt 20x20.txt

in addition the provided script test.sh can be executed with the command sbatch which 
runs all the provided test files and prints their results for different number of processes.

