#!/bin/bash
sbatch <<EOT
#!/bin/sh
#SBATCH --job-name=efoeqof
#SBATCH --output=slurm/output/$1_$3.txt
#SBATCH --error=slurm/error/$1_$3.txt
#SBATCH --ntasks=$1
#SBATCH --cpus-per-task=4
#SBATCH -x lab2p[1-20]
#SBATCH --mem=4G

srun life3d $2 $3 $4 $5

EOT