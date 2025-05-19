#!/bin/bash 
#SBATCH -J U_500
#SBATCH -o U_500_ds.o%j 
#SBATCH -e U_500_ds.o%j.err
#SBATCH --mail-user=tpcarroll@gmail.com
#SBATCH --mail-type=FAIL,END,BEGIN
#SBATCH --ntasks-per-node=28 -N 1
#SBATCH -t 15:30:00 
#SBATCH --mem-per-cpu=1GB

COUNT=10000
UTILIZATION=500

module load intel-oneapi/2025.1.0
module load OpenMPI/5.0.3-intel-compilers-2023.1.0 


mpiicpx -o main main.cpp

mpirun ./main $COUNT $UTILIZATION