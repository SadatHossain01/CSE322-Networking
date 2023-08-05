# • Number of nodes: 20, 40, 60, 80, 100.
# • Number of flows: 10, 20, 30, 40, and 50 (Not considering the Ack flows)
# • Number of packets per second: 100, 200, 300, 400, 500
# • Speed of nodes: 5 m/s, 10 m/s, 15 m/s, 20 m/s, 25 m/s (Only in wireless mobile topology)
# • Coverage area : 1/2/4/5 * Tx range (Only in wireless static topology)

#!/bin/bash

# Define the ranges for each parameter
numNodes_range="20 40 60 80 100"
numFlows_range="10 20 30 40 50"
numPackets_range="100 200 300 400 500"
coverage_range="1 2 4 5"

file_name="task_stat"

rm -rf scratch/results/static
rm -f what1.dat
rm -f what2.dat

# Create a directory to store the results
mkdir -p scratch/results/static

plot() {
    source_file=$1
    xlabel=$2
    ylabel=$3
    title=$4
    output_file=${source_file%.dat}
    output_file="$output_file.png"

    # set xlabel "$xlabel"
    # set ylabel "$ylabel"

    gnuplot -persist <<-EOFMarker
        set datafile separator " "
        set title "$title"
        set terminal png size 640,480
        set output "$output_file"
        plot "$source_file" using 1:2 with linespoints
EOFMarker
}

# Function to run the simulation an d collect results for a specific parameter
run_simulation() {
    param_to_vary=$1
    output_file1="scratch/results/static/${param_to_vary}_Throughput.dat"
    output_file2="scratch/results/static/${param_to_vary}_Ratio.dat"

    case "$param_to_vary" in
    "Node")
        param_values=$numNodes_range
        ;;
    "Flow")
        param_values=$numFlows_range
        ;;
    "Packet")
        param_values=$numPackets_range
        ;;
    "Coverage")
        param_values=$coverage_range
        ;;
    *)
        echo "Unknown parameter to vary: $param_to_vary"
        exit 1
        ;;
    esac

    # Loop over the parameter values and run the simulation for each value
    str1="./ns3 run '$file_name' --"
    for value in $param_values; do
        case "$param_to_vary" in
        "Node")
            str="$str1 --nNodes=$value"
            ;;
        "Flow")
            str="$str1 --nFlows=$value"
            ;;
        "Packet")
            str="$str1 --nPackets=$value"
            ;;
        "Coverage")
            str="$str1 --coverageArea=$value"
            ;;
        esac

        # Run the simulation with the given parameters and save the output to the output file
        echo -n $value >>what1.dat
        echo -n $value >>what2.dat
        eval $str
    done

    cp what1.dat $output_file1
    cp what2.dat $output_file2
    rm -f what1.dat
    rm -f what2.dat
    echo "Simulation for parameter: $param_to_vary completed. Results are saved in $output_file1 and $output_file2"

    # Plot the results
    xlabel="None"
    title1="None"
    title2="None"
    case "$param_to_vary" in
    "Node")
        xlabel="Number of nodes"
        title1="Node vs Throughput"
        title2="Node vs Delivery Ratio"
        ;;
    "Flow")
        xlabel="Number of flows"
        title1="Flow vs Throughput"
        title2="Flow vs Delivery Ratio"
        ;;
    "Packet")
        xlabel="Number of packets per second"
        title1="Packet vs Throughput"
        title2="Packet vs Delivery Ratio"
        ;;
    "Coverage")
        xlabel="Coverage area multiplier"
        title1="Coverage Area vs Throughput"
        title2="Coverage Area vs Delivery Ratio"
        ;;
    esac
    plot $output_file1 "$xlabel" "Throughput (Mbps)" "$title1"
    plot $output_file2 "$xlabel" "Delivery Ratio" "$title2"
}

# Call the function with the parameter to vary
run_simulation "Node"
run_simulation "Flow"
run_simulation "Packet"
run_simulation "Coverage"
