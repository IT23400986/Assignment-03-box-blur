#!/bin/bash

# Automated Benchmark Script
# Runs all implementations and creates performance graphs

echo "======================================"
echo "  Box Blur Performance Benchmark"
echo "======================================"
echo ""

# Check if input image is provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <input_image.jpg>"
    echo ""
    echo "Example: $0 photo.jpg"
    exit 1
fi

INPUT_IMAGE=$1

# Check if image exists
if [ ! -f "$INPUT_IMAGE" ]; then
    echo "Error: Image '$INPUT_IMAGE' not found!"
    exit 1
fi

# Get image info
echo "Input image: $INPUT_IMAGE"
echo ""

# Create output directory
mkdir -p results/benchmark_outputs
RESULTS_FILE="results/benchmark_results.txt"
CSV_FILE="results/benchmark_results.csv"
NUM_RUNS=5

# Clear previous results
> $RESULTS_FILE
echo "Implementation,Time(s),Pixels,Speed(Mpx/s)" > $CSV_FILE

echo "Running benchmarks..."
echo "===================="
echo ""

# Function to extract metrics from output
extract_time() {
    grep -i "time:" | sed 's/.*: //' | sed 's/ seconds//' | head -1
}

extract_pixels() {
    grep -i "pixels" | grep -v "Mpixels" | sed 's/.*: //' | head -1
}

extract_speed() {
    grep -i "speed:\|throughput:" | sed 's/.*: //' | sed 's/ Mpixels\/sec//' | head -1
}

# 1. Serial
echo "[1/5] Serial..."
if [ -f "./serial_box_blur" ]; then
    TOTAL_TIME=0
    for RUN in $(seq 1 $NUM_RUNS); do
        echo "  Run $RUN/$NUM_RUNS..."
        OUTPUT=$(./serial_box_blur "$INPUT_IMAGE" results/benchmark_outputs/serial.jpg 2>&1)
        TIME=$(echo "$OUTPUT" | extract_time)
        TOTAL_TIME=$(echo "$TOTAL_TIME + $TIME" | bc)
    done
    AVG_TIME=$(echo "scale=6; $TOTAL_TIME / $NUM_RUNS" | bc)
    echo "$OUTPUT" | tee -a $RESULTS_FILE
    PIXELS=$(echo "$OUTPUT" | extract_pixels)
    SPEED=$(echo "scale=2; $PIXELS / ($AVG_TIME * 1000000)" | bc)
    echo "Serial,$AVG_TIME,$PIXELS,$SPEED" >> $CSV_FILE
    echo "  Average time: ${AVG_TIME}s (over $NUM_RUNS runs)"
    echo ""
else
    echo "  SKIP: serial_box_blur not found"
    echo ""
fi

# 2. OpenMP (with different thread counts)
echo "[2/5] OpenMP (multi-threaded)..."
if [ -f "./openmp_box_blur" ]; then
    for THREADS in 1 2 3 4 5 6 7 8; do
        echo "  Testing with $THREADS thread(s)..."
        export OMP_NUM_THREADS=$THREADS
        TOTAL_TIME=0
        for RUN in $(seq 1 $NUM_RUNS); do
            echo "    Run $RUN/$NUM_RUNS..."
            OUTPUT=$(./openmp_box_blur "$INPUT_IMAGE" results/benchmark_outputs/openmp_${THREADS}t.jpg 2>&1)
            TIME=$(echo "$OUTPUT" | extract_time)
            TOTAL_TIME=$(echo "$TOTAL_TIME + $TIME" | bc)
        done
        AVG_TIME=$(echo "scale=6; $TOTAL_TIME / $NUM_RUNS" | bc)
        echo "$OUTPUT" | tee -a $RESULTS_FILE
        PIXELS=$(echo "$OUTPUT" | extract_pixels)
        SPEED=$(echo "scale=2; $PIXELS / ($AVG_TIME * 1000000)" | bc)
        echo "OpenMP-${THREADS}T,$AVG_TIME,$PIXELS,$SPEED" >> $CSV_FILE
        echo "    Average time: ${AVG_TIME}s (over $NUM_RUNS runs)"
        echo ""
    done
else
    echo "  SKIP: openmp_box_blur not found"
    echo ""
fi

# 3. MPI (with different process counts)
echo "[3/5] MPI (distributed)..."
if [ -f "./mpi_box_blur" ]; then
    for PROCS in 1 2 3 4; do
        echo "  Testing with $PROCS process(es)..."
        TOTAL_TIME=0
        for RUN in $(seq 1 $NUM_RUNS); do
            echo "    Run $RUN/$NUM_RUNS..."
            OUTPUT=$(mpirun -np $PROCS ./mpi_box_blur "$INPUT_IMAGE" results/benchmark_outputs/mpi_${PROCS}p.jpg 2>&1)
            TIME=$(echo "$OUTPUT" | extract_time)
            TOTAL_TIME=$(echo "$TOTAL_TIME + $TIME" | bc)
        done
        AVG_TIME=$(echo "scale=6; $TOTAL_TIME / $NUM_RUNS" | bc)
        echo "$OUTPUT" | tee -a $RESULTS_FILE
        PIXELS=$(echo "$OUTPUT" | extract_pixels)
        SPEED=$(echo "scale=2; $PIXELS / ($AVG_TIME * 1000000)" | bc)
        echo "MPI-${PROCS}P,$AVG_TIME,$PIXELS,$SPEED" >> $CSV_FILE
        echo "    Average time: ${AVG_TIME}s (over $NUM_RUNS runs)"
        echo ""
    done
else
    echo "  SKIP: mpi_box_blur not found (run 'make mpi' to build)"
    echo ""
fi

# 4. OpenCL
echo "[4/5] OpenCL (GPU - AMD)..."
if [ -f "./opencl_box_blur" ]; then
    OUTPUT=$(./opencl_box_blur "$INPUT_IMAGE" results/benchmark_outputs/opencl.jpg 2>&1)
    echo "$OUTPUT"
    
    # Check if OpenCL ran successfully (no error message)
    if echo "$OUTPUT" | grep -q "Error during operation"; then
        echo "  SKIP: OpenCL failed (GPU drivers not available)"
        echo ""
    else
        TOTAL_TIME=0
        for RUN in $(seq 1 $NUM_RUNS); do
            echo "  Run $RUN/$NUM_RUNS..."
            OUTPUT=$(./opencl_box_blur "$INPUT_IMAGE" results/benchmark_outputs/opencl.jpg 2>&1)
            TIME=$(echo "$OUTPUT" | extract_time)
            TOTAL_TIME=$(echo "$TOTAL_TIME + $TIME" | bc)
        done
        AVG_TIME=$(echo "scale=6; $TOTAL_TIME / $NUM_RUNS" | bc)
        echo "$OUTPUT" | tee -a $RESULTS_FILE
        PIXELS=$(echo "$OUTPUT" | extract_pixels)
        SPEED=$(echo "scale=2; $PIXELS / ($AVG_TIME * 1000000)" | bc)
        echo "OpenCL,$AVG_TIME,$PIXELS,$SPEED" >> $CSV_FILE
        echo "  Average time: ${AVG_TIME}s (over $NUM_RUNS runs)"
        echo ""
    fi
else
    echo "  SKIP: opencl_box_blur not found"
    echo ""
fi

# 5. CUDA
echo "[5/5] CUDA (GPU - NVIDIA)..."
if [ -f "./cuda_box_blur" ]; then
    TOTAL_TIME=0
    for RUN in $(seq 1 $NUM_RUNS); do
        echo "  Run $RUN/$NUM_RUNS..."
        OUTPUT=$(./cuda_box_blur "$INPUT_IMAGE" results/benchmark_outputs/cuda.jpg 2>&1)
        TIME=$(echo "$OUTPUT" | extract_time)
        TOTAL_TIME=$(echo "$TOTAL_TIME + $TIME" | bc)
    done
    AVG_TIME=$(echo "scale=6; $TOTAL_TIME / $NUM_RUNS" | bc)
    echo "$OUTPUT" | tee -a $RESULTS_FILE
    PIXELS=$(echo "$OUTPUT" | extract_pixels)
    SPEED=$(echo "scale=2; $PIXELS / ($AVG_TIME * 1000000)" | bc)
    echo "CUDA,$AVG_TIME,$PIXELS,$SPEED" >> $CSV_FILE
    echo "  Average time: ${AVG_TIME}s (over $NUM_RUNS runs)"
    echo ""
else
    echo "  SKIP: cuda_box_blur not found (requires NVIDIA GPU)"
    echo ""
fi

echo "======================================"
echo "Benchmark complete!"
echo "Results saved to: $RESULTS_FILE"
echo "CSV data: $CSV_FILE"
echo ""

# Create text summary
echo "Creating performance summary..."
SUMMARY_FILE="results/performance_summary.txt"

echo "======================================================================" > $SUMMARY_FILE
echo "             BOX BLUR PERFORMANCE SUMMARY" >> $SUMMARY_FILE
echo "======================================================================" >> $SUMMARY_FILE
echo "" >> $SUMMARY_FILE
echo "Input Image: $INPUT_IMAGE" >> $SUMMARY_FILE
echo "" >> $SUMMARY_FILE

# Read CSV and create table
printf "%-15s %-12s %-18s %-12s\n" "Implementation" "Time (s)" "Speed (Mpx/s)" "Speedup" >> $SUMMARY_FILE
echo "----------------------------------------------------------------------" >> $SUMMARY_FILE

# Get serial time for speedup calculation
SERIAL_TIME=$(awk -F, 'NR==2 {print $2}' $CSV_FILE)

tail -n +2 $CSV_FILE | while IFS=, read impl time pixels speed; do
    if [ ! -z "$SERIAL_TIME" ] && [ "$SERIAL_TIME" != "0" ]; then
        SPEEDUP=$(echo "scale=2; $SERIAL_TIME / $time" | bc)
    else
        SPEEDUP="1.00"
    fi
    printf "%-15s %-12s %-18s %-12s\n" "$impl" "$time" "$speed" "${SPEEDUP}x" >> $SUMMARY_FILE
done

echo "" >> $SUMMARY_FILE
echo "======================================================================" >> $SUMMARY_FILE
echo "KEY FINDINGS:" >> $SUMMARY_FILE
echo "======================================================================" >> $SUMMARY_FILE

# Find fastest
FASTEST=$(tail -n +2 $CSV_FILE | sort -t, -k2 -n | head -1)
FASTEST_IMPL=$(echo $FASTEST | cut -d, -f1)
FASTEST_TIME=$(echo $FASTEST | cut -d, -f2)

echo "• Fastest implementation: $FASTEST_IMPL ($FASTEST_TIME seconds)" >> $SUMMARY_FILE

if [ ! -z "$SERIAL_TIME" ] && [ "$SERIAL_TIME" != "0" ] && [ "$FASTEST_TIME" != "0" ]; then
    BEST_SPEEDUP=$(echo "scale=2; $SERIAL_TIME / $FASTEST_TIME" | bc)
    echo "• Best speedup: ${BEST_SPEEDUP}x faster than serial" >> $SUMMARY_FILE
fi

echo "" >> $SUMMARY_FILE

cat $SUMMARY_FILE

echo ""
echo "Summary saved to: $SUMMARY_FILE"
echo ""

# Try to generate graph if matplotlib is available
if command -v python3 &> /dev/null; then
    if python3 -c "import matplotlib" 2>/dev/null; then
        echo "Generating performance graphs..."
        python3 scripts/plot_results.py "$CSV_FILE"
    else
        echo "Note: Install matplotlib to generate graphs: pip3 install matplotlib"
    fi
fi

echo ""
echo "Done! Check the results folder."
