#!/bin/bash

TempFile1="what1.dat"
TempFile2="what2.dat"
TempFile3="what3.dat"

DataRateValues=(1 25 50 75 100 125 150 175 200 225 250 275 300)
PacketLossExponentValues=(-2 -3 -4 -5 -6)

SourceFile="scratch/1905001.cc"

Algo1="TcpNewReno"
OtherAlgos=("TcpAdaptiveReno" "TcpWestwoodPlus" "TcpHighSpeed")

# Data Files
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
    "scratch/results/data/TcpNewReno_AdaptiveReno_Time_CongestionWindow.dat"
    "scratch/results/data/TcpNewReno_WestwoodPlus_Time_CongestionWindow.dat"
    "scratch/results/data/TcpNewReno_HighSpeed_Time_CongestionWindow.dat"
)
CongestionOtherFiles=(
    "scratch/results/data/TcpAdaptiveReno_Time_CongestionWindow.dat"
    "scratch/results/data/TcpWestwoodPlus_Time_CongestionWindow.dat"
    "scratch/results/data/TcpHighSpeed_Time_CongestionWindow.dat"
)
JainLossFiles=(
    "scratch/results/data/TcpNewReno_AdaptiveReno_Loss_JainIndex.dat"
    "scratch/results/data/TcpNewReno_WestwoodPlus_Loss_JainIndex.dat"
    "scratch/results/data/TcpNewReno_HighSpeed_Loss_JainIndex.dat"
)
JainDataFiles=(
    "scratch/results/data/TcpNewReno_AdaptiveReno_DataRate_JainIndex.dat"
    "scratch/results/data/TcpNewReno_WestwoodPlus_DataRate_JainIndex.dat"
    "scratch/results/data/TcpNewReno_HighSpeed_DataRate_JainIndex.dat"
)

# Plot Files
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
JainLossPlotFiles=(
    "scratch/results/plots/TcpNewReno_AdaptiveReno_Loss_JainIndex.png"
    "scratch/results/plots/TcpNewReno_WestwoodPlus_Loss_JainIndex.png"
    "scratch/results/plots/TcpNewReno_HighSpeed_Loss_JainIndex.png"
)
JainDataPlotFiles=(
    "scratch/results/plots/TcpNewReno_AdaptiveReno_DataRate_JainIndex.png"
    "scratch/results/plots/TcpNewReno_WestwoodPlus_DataRate_JainIndex.png"
    "scratch/results/plots/TcpNewReno_HighSpeed_DataRate_JainIndex.png"
)

runSimulation() {
    local paramToVary="$1"
    local expName="$2"
    local algo1="$3"
    local algo2="$4"
    local outFile1="$5"
    local outFile2="$6"
    local outFile3="$7" # used for Jain's Fairness Index
    shift 7
    local paramValues=("$@")

    for value in "${paramValues[@]}"; do
        local str="./ns3 run ${SourceFile} -- --${paramToVary}=${value} --experimentName=${expName} --algorithm1=${algo1} --algorithm2=${algo2}"
        echo "$str"
        eval "$str"
    done

    # Copy the contents of the temporary output files to the actual output files
    cat "$TempFile1" >"$outFile1"
    cat "$TempFile2" >"$outFile2"
    cat "$TempFile3" >"$outFile3" # used for Jain's Fairness Index

    # Clear the temporary output files
    >"$TempFile1"
    >"$TempFile2"
    >"$TempFile3"
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

plotDoubleFile() {
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
        plot "$file1" using 1:2 title "$algo1" with lines lc "blue", \
             "$file2" using 1:2 title "$algo2" with lines lc "red"
EOFMarker
}

plotSingleFile() {
    local file="$1"
    local x_title="$2"
    local y_title="$3"
    local title="$4"
    local output_file="$5"

    gnuplot -persist <<-EOFMarker
        set datafile separator " "
        set terminal png size 640,480
        set xlabel "$x_title"
        set ylabel "$y_title"
        set grid
        set output "$output_file"
        plot "$file" using 1:2 title "$title" with lines
EOFMarker
}

cd ..
rm -rf scratch/results
rm -f "$TempFile1"
rm -f "$TempFile2"
rm -f "$TempFile3"
mkdir -p scratch/results/data
mkdir -p scratch/results/plots

# Create the .dat files first so that GLOB mismatch does not happen
touch "$TempFile1"
touch "$TempFile2"
touch "$TempFile3"

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
for file in "${JainLossFiles[@]}"; do
    touch "$file"
done
for file in "${JainDataFiles[@]}"; do
    touch "$file"
done

# Throughput (and Jane's Index) vs Bottleneck Link Capacity
for ((i = 0; i < ${#OtherAlgos[@]}; ++i)); do
    runSimulation "bottleneckBW" "data" "$Algo1" "${OtherAlgos[$i]}" "${DataRateNewRenoFiles[$i]}" "${DataRateOtherFiles[$i]}" "${JainDataFiles[$i]}" "${DataRateValues[@]}"
done

# Throughput (and Jane's Index) vs Packet Loss Exponent
for ((i = 0; i < ${#OtherAlgos[@]}; ++i)); do
    runSimulation "packetLossExponent" "loss" "$Algo1" "${OtherAlgos[$i]}" "${PacketLossNewRenoFiles[$i]}" "${PacketLossOtherFiles[$i]}" "${JainLossFiles[$i]}" "${PacketLossExponentValues[@]}"
done

# Congestion Window vs Time
for ((i = 0; i < ${#OtherAlgos[@]}; ++i)); do
    runSimulationForCongestionWindow "$Algo1" "${OtherAlgos[$i]}" "${CongestionNewRenoFiles[$i]}" "${CongestionOtherFiles[$i]}"
done

# Throughput vs Bottleneck Link Capacity Plot
for ((i = 0; i < ${#OtherAlgos[@]}; ++i)); do
    plotDoubleFile "${DataRateNewRenoFiles[$i]}" "${DataRateOtherFiles[$i]}" "$Algo1" "${OtherAlgos[$i]}" "Bottleneck Link Capacity (Mbps)" "Throughput (kbps)" "${DataRatePlotFiles[$i]}"
done

# Throughput vs Packet Loss Exponent Plot
for ((i = 0; i < ${#OtherAlgos[@]}; ++i)); do
    plotDoubleFile "${PacketLossNewRenoFiles[$i]}" "${PacketLossOtherFiles[$i]}" "$Algo1" "${OtherAlgos[$i]}" "Packet Loss Exponent" "Throughput (kbps)" "${PacketLossPlotFiles[$i]}"
done

# Congestion Window vs Time Plot
for ((i = 0; i < ${#OtherAlgos[@]}; ++i)); do
    plotDoubleFile "${CongestionNewRenoFiles[$i]}" "${CongestionOtherFiles[$i]}" "$Algo1" "${OtherAlgos[$i]}" "Time (seconds)" "Congestion Window Size (kB)" "${CongestionPlotFiles[$i]}"
done

# Jain's Fairness Index vs Packet Loss Exponent Plot
for ((i = 0; i < ${#OtherAlgos[@]}; ++i)); do
    plotSingleFile "${JainLossFiles[$i]}" "Packet Loss Exponent" "Jain's Fairness Index" "$Algo1-${OtherAlgos[$i]}" "${JainLossPlotFiles[$i]}"
done

# Jain's Fairness Index vs Bottleneck Link Capacity Plot
for ((i = 0; i < ${#OtherAlgos[@]}; ++i)); do
    plotSingleFile "${JainDataFiles[$i]}" "Bottleneck Link Capacity (Mbps)" "Jain's Fairness Index" "$Algo1-${OtherAlgos[$i]}" "${JainDataPlotFiles[$i]}"
done

rm -f "$TempFile1"
rm -f "$TempFile2"
rm -f "$TempFile3"
cd scratch
