# /usr/bin/env bash

# Compile the program
gcc global-align.c -o global-align

# Hand it a simple input
echo "Handling a simple input..."
./global-align vintners writers --scores=M20I-1D-1R-1 --type=max

# Same input, different parameters
echo "Same input, different parameters..."
./global-align vintners writers --scores=M-10I10D10R5 --type=min

# Give it a more complex input
#global-align TTTTGAGGTGCAGATAGCTTGCTTTATTTTGTTGTTACTATCTCAAGGA GAGACAGAGTCTCACTCTGTTGCACAGGCTGGAGTGCAGTGGCACAATC --scores=M-10I10D10R5 --type=min
