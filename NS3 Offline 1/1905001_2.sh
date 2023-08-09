#!/bin/bash

# Define the ranges for each parameter
numNodes_array=(20 40 60 80 100)
numFlows_array=(10 20 30 40 50)
numPackets_array=(100 200 300 400 500)
speed_array=(5 10 15 20 25)

file_name="1905001_2"

cd ..
rm -rf scratch/results/mobile
rm -f what1.dat
rm -f what2.dat

# Create a directory to store the results
mkdir -p scratch/results/mobile

plot() {
    local source_file="$1"
    local title="$2"
    local output_file=${source_file%.dat}
    local output_file="$output_file.png"

    gnuplot -persist <<-EOFMarker
        set datafile separator " "
        set terminal png size 640,480
        set output "$output_file"
        plot "$source_file" using 1:2 title "$title" with linespoints
EOFMarker
}

# Function to run the simulation an d collect results for a specific parameter
run_simulation() {
    local parameter="$1"
    local param_command="$2"
    shift 2
    local param_values=("$@")
    local output_file1="scratch/results/mobile/${parameter}_Throughput.dat"
    local output_file2="scratch/results/mobile/${parameter}_Ratio.dat"

    # Loop over the parameter values and run the simulation for each value
    local str1="./ns3 run ${file_name} --"
    for value in "${param_values[@]}"; do
        str="$str1 --${param_command}=$value"

        # Run the simulation with the given parameters and save the output to the output file
        echo -n $value >>what1.dat
        echo -n $value >>what2.dat
        echo "$str"
        eval "$str"
    done

    cp what1.dat "$output_file1"
    cp what2.dat "$output_file2"
    rm -f what1.dat
    rm -f what2.dat
    echo "Done running simulation for ${parameter}"

    plot "$output_file1" "Throughput vs ${parameter}"
    plot "$output_file2" "Ratio vs ${parameter}"
}

# Call the function with the parameter to vary
run_simulation "Node" "nNodes" "${numNodes_array[@]}"
run_simulation "Flow" "nFlows" "${numFlows_array[@]}"
run_simulation "Packets Per Second" "nPackets" "${numPackets_array[@]}"
run_simulation "Speed(m/s)" "speed" "${speed_array[@]}"

cd scratch
