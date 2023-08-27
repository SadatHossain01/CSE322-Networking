#!/bin/bash

TempFile1="what1.dat"
TempFile2="what2.dat"
Algo1="TcpNewReno"
DataRateValues=(1 50 100 150 200 250 300)
PacketLossExponentValues=(-2 -3 -4 -5 -6)
SourceFile="scratch/1905001.cc"

OtherAlgos=("TcpAdaptiveReno" "TcpWestwoodPlus" "TcpHighSpeed")
PacketLossNewRenoFiles=(
    "scratch/results/data/TcpNewReno_AdaptiveReno_Loss_Throughput.dat"
    "scratch/results/data/TcpNewReno_WestwoodPlus_Loss_Throughput.dat"
    "scratch/results/data/TcpNewReno_HighSpeed_Loss_Throughput.dat"
)
PacketLossOtherFiles=(
    "scratch/results/data/TcpAdaptiveReno_Loss_Throughput.dat"
    "scratch/results/data/TcpWestwoodPlus_Loss_Throughput.dat"
    "scratch/results/data/TcpHighSpeed_Loss_Throughput.dat"
)
DataRateNewRenoFiles=(
    "scratch/results/data/TcpNewReno_AdaptiveReno_DataRate_Throughput.dat"
    "scratch/results/data/TcpNewReno_WestwoodPlus_DataRate_Throughput.dat"
    "scratch/results/data/TcpNewReno_HighSpeed_DataRate_Throughput.dat"
)
DataRateOtherFiles=(
    "scratch/results/data/TcpAdaptiveReno_DataRate_Throughput.dat"
    "scratch/results/data/TcpWestwoodPlus_DataRate_Throughput.dat"
    "scratch/results/data/TcpHighSpeed_DataRate_Throughput.dat"
)
CongestionNewRenoFiles=(
    "scratch/results/data/TcpNewReno_AdaptiveReno_CongestionWindow_Time.dat"
    "scratch/results/data/TcpNewReno_WestwoodPlus_CongestionWindow_Time.dat"
    "scratch/results/data/TcpNewReno_HighSpeed_CongestionWindow_Time.dat"
)
CongestionOtherFiles=(
    "scratch/results/data/TcpAdaptiveReno_Time_CongestionWindow.dat"
    "scratch/results/data/TcpWestwoodPlus_Time_CongestionWindow.dat"
    "scratch/results/data/TcpHighSpeed_Time_CongestionWindow.dat"
)
DataRatePlotFiles=(
    "scratch/results/plots/TcpNewReno_AdaptiveReno_DataRate_Throughput.png"
    "scratch/results/plots/TcpNewReno_WestwoodPlus_DataRate_Throughput.png"
    "scratch/results/plots/TcpNewReno_HighSpeed_DataRate_Throughput.png"
)
PacketLossPlotFiles=(
    "scratch/results/plots/TcpNewReno_AdaptiveReno_Loss_Throughput.png"
    "scratch/results/plots/TcpNewReno_WestwoodPlus_Loss_Throughput.png"
    "scratch/results/plots/TcpNewReno_HighSpeed_Loss_Throughput.png"
)
CongestionPlotFiles=(
    "scratch/results/plots/TcpNewReno_AdaptiveReno_Time_CongestionWindow.png"
    "scratch/results/plots/TcpNewReno_WestwoodPlus_Time_CongestionWindow.png"
    "scratch/results/plots/TcpNewReno_HighSpeed_Time_CongestionWindow.png"
)

runSimulation() {
    local paramToVary="$1"
    local expName="$2"
    local algo1="$3"
    local algo2="$4"
    local outFile1="$5"
    local outFile2="$6"
    shift 6
    local paramValues=("$@")

    for value in "${paramValues[@]}"; do
        local str="./ns3 run ${SourceFile} -- --${paramToVary}=${value} --experimentName=${expName} --algorithm1=${algo1} --algorithm2=${algo2}"
        echo "$str"
        eval "$str"
    done

    # Copy the contents of the temporary output files to the actual output files
    cat "$TempFile1" >"$outFile1"
    cat "$TempFile2" >"$outFile2"

    # Clear the temporary output files
    >"$TempFile1"
    >"$TempFile2"
}

runSimulationForCongestionWindow() {
    local expName="congestion"
    local algo1="$1"
    local algo2="$2"
    local outFile1="$3"
    local outFile2="$4"

    local str="./ns3 run ${SourceFile} -- --experimentName=${expName} --algorithm1=${algo1} --algorithm2=${algo2}"
    echo "$str"
    eval "$str"

    # Copy the contents of the temporary output files to the actual output files
    cat "$TempFile1" >"$outFile1"
    cat "$TempFile2" >"$outFile2"

    # Clear the temporary output files
    >"$TempFile1"
    >"$TempFile2"
}

plot() {
    local file1="$1"
    local file2="$2"
    local algo1="$3"
    local algo2="$4"
    local x_title="$5"
    local y_title="$6"
    local output_file="$7"

    gnuplot -persist <<-EOFMarker
        set datafile separator " "
        set terminal png size 640,480
        set xlabel "$x_title"
        set ylabel "$y_title"
        set grid
        set output "$output_file"
        plot "$file1" using 1:2 title "$algo1" with linespoints lc "blue", \
             "$file2" using 1:2 title "$algo2" with linespoints lc "red"
EOFMarker
}

cd ..
rm -rf scratch/results
rm -f "$TempFile1"
rm -f "$TempFile2"
mkdir -p scratch/results/data
mkdir -p scratch/results/plots

# Create the .dat files first so that GLOB mismatch does not happen
touch "$TempFile1"
touch "$TempFile2"
for file in "${PacketLossNewRenoFiles[@]}"; do
    touch "$file"
done
for file in "${PacketLossOtherFiles[@]}"; do
    touch "$file"
done
for file in "${DataRateNewRenoFiles[@]}"; do
    touch "$file"
done
for file in "${DataRateOtherFiles[@]}"; do
    touch "$file"
done
for file in "${CongestionNewRenoFiles[@]}"; do
    touch "$file"
done
for file in "${CongestionOtherFiles[@]}"; do
    touch "$file"
done

# Data Rate Experiment
for ((i = 0; i < ${#OtherAlgos[@]}; ++i)); do
    runSimulation "bottleneckBW" "data" "$Algo1" "${OtherAlgos[$i]}" "${DataRateNewRenoFiles[$i]}" "${DataRateOtherFiles[$i]}" "${DataRateValues[@]}"
done

# Packet Loss Experiment
for ((i = 0; i < ${#OtherAlgos[@]}; ++i)); do
    runSimulation "packetLossExponent" "loss" "$Algo1" "${OtherAlgos[$i]}" "${PacketLossNewRenoFiles[$i]}" "${PacketLossOtherFiles[$i]}" "${PacketLossExponentValues[@]}"
done

# Congestion Window Experiment
for ((i = 0; i < ${#OtherAlgos[@]}; ++i)); do
    runSimulationForCongestionWindow "$Algo1" "${OtherAlgos[$i]}" "${CongestionNewRenoFiles[$i]}" "${CongestionOtherFiles[$i]}"
done

# Data Rate Plot
for ((i = 0; i < ${#OtherAlgos[@]}; ++i)); do
    plot "${DataRateNewRenoFiles[$i]}" "${DataRateOtherFiles[$i]}" "$Algo1" "${OtherAlgos[$i]}" "Bottleneck Link Capacity (Mbps)" "Throughput (kbps)" "${DataRatePlotFiles[$i]}"
done

# Packet Loss Plot
for ((i = 0; i < ${#OtherAlgos[@]}; ++i)); do
    plot "${PacketLossNewRenoFiles[$i]}" "${PacketLossOtherFiles[$i]}" "$Algo1" "${OtherAlgos[$i]}" "Packet Loss Exponent" "Throughput (kbps)" "${PacketLossPlotFiles[$i]}"
done

# Congestion Window Plot
for ((i = 0; i < ${#OtherAlgos[@]}; ++i)); do
    plot "${CongestionNewRenoFiles[$i]}" "${CongestionOtherFiles[$i]}" "$Algo1" "${OtherAlgos[$i]}" "Time (seconds)" "Congestion Window Size (kB)" "${CongestionPlotFiles[$i]}"
done

rm -f "$TempFile1"
rm -f "$TempFile2"
cd scratch
