#!/bin/bash 
#SBATCH -J R0.7_1000
#SBATCH -o U_1000_ds.o%j 
#SBATCH -e U_1000_ds.o%j.err
#SBATCH --mail-user=tpcarroll@gmail.com
#SBATCH --mail-type=FAIL,END,BEGIN
#SBATCH --ntasks-per-node=28 -N 1
#SBATCH -t 48:00:00 
#SBATCH --mem-per-cpu=1GB

COUNT=10000
UTILIZATION=1000

module load intel-oneapi/2025.1.0
module load OpenMPI/5.0.3-intel-compilers-2023.1.0 


mpiicpx -o main main.cpp

mpirun ./main $COUNT $UTILIZATION