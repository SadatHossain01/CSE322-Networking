#!/bin/bash

# Define the ranges for each parameter
numNodes_array=(20 40 60 80 100)
numFlows_array=(10 20 30 40 50)
numPackets_array=(100 200 300 400 500)
coverage_array=(1 2 4 5)

file_name="1905001_1"
temp_output_file="what.dat"
type="static"

# Define the output files
Node_files=("scratch/results/${type}/Node_Throughput.dat" "scratch/results/${type}/Node_Ratio.dat")
Flow_files=("scratch/results/${type}/Flow_Throughput.dat" "scratch/results/${type}/Flow_Ratio.dat")
Packets_files=("scratch/results/${type}/Packets_Throughput.dat" "scratch/results/${type}/Packets_Ratio.dat")
Coverage_files=("scratch/results/${type}/Coverage_Throughput.dat" "scratch/results/${type}/Coverage_Ratio.dat")

# Function to plot the results using gnuplot
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

# Function to run the simulation and collect results for a specific parameter
run_simulation() {
    local param_command="$1"
    shift 1
    local output_files=("$@")
    shift 2 # size of output_files is 2
    local param_values=("$@")
    local output_file1="${output_files[0]}"
    local output_file2="${output_files[1]}"

    # echo "$output_file1" "$output_file2"

    # Loop over the parameter values and run the simulation for each value
    local str1="./ns3 run ${file_name} --"
    for value in "${param_values[@]}"; do
        str="$str1 --${param_command}=$value"

        # Run the simulation with the given parameters and save the output to the output file
        echo "$str"
        eval "$str"
        # temp_output_file's first line is throughput, second line is delivery ratio
        throughput=$(head -n 1 "$temp_output_file")
        ratio=$(tail -n 1 "$temp_output_file")
        # echo "Throughput: $res1" "Delivery Ratio: $res2"
        echo "$value" "$throughput" >>"$output_file1"
        echo "$value" "$ratio" >>"$output_file2"
        >"$temp_output_file" # clear the temp file
    done

    echo "Done running simulation for ${param_command}"
}

cd ..
rm -rf scratch/results/${type}
rm -f "$temp_output_file"
touch "$temp_output_file"

# Create a directory to store the results
mkdir -p scratch/results/${type}

# Create the output files in advance to avoid GLOB mismatch error
touch "${Node_files[@]}"
touch "${Flow_files[@]}"
touch "${Packets_files[@]}"
touch "${Coverage_files[@]}"

# Simulation
run_simulation "nNodes" "${Node_files[@]}" "${numNodes_array[@]}"
run_simulation "nFlows" "${Flow_files[@]}" "${numFlows_array[@]}"
run_simulation "nPackets" "${Packets_files[@]}" "${numPackets_array[@]}"
run_simulation "coverageAreaMultiplier" "${Coverage_files[@]}" "${coverage_array[@]}"

# Plotting
plot "${Node_files[0]}" "Throughput vs Node"
plot "${Node_files[1]}" "Delivery Ratio vs Node"

plot "${Flow_files[0]}" "Throughput vs Flow"
plot "${Flow_files[1]}" "Delivery Ratio vs Flow"

plot "${Packets_files[0]}" "Throughput vs Packets Per Second"
plot "${Packets_files[1]}" "Delivery Ratio vs Packets Per Second"

plot "${Coverage_files[0]}" "Throughput vs Coverage Area Multiplier"
plot "${Coverage_files[1]}" "Delivery Ratio vs Coverage Area Multiplier"

rm -f "$temp_output_file"
cd scratch
