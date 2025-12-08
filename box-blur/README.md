# Box Blur Parallel Comparison

This project implements and compares the performance of the Box blur algorithm using **five different approaches**: Serial, MPI, OpenMP, OpenCL, and CUDA. Each implementation is designed to process images and measure execution time to evaluate the speedup achieved through parallelization.

## Implementation Overview

- **Serial**: Single-threaded CPU implementation (baseline)
- **MPI**: Distributed computing using Message Passing Interface
- **OpenMP**: Multi-threaded CPU parallelism using shared memory
- **OpenCL**: GPU acceleration (works with AMD Radeon, NVIDIA, Intel GPUs)
- **CUDA**: GPU acceleration (NVIDIA GPUs only)

## Project Structure

```
box-blur/
├── src/
│   ├── serial/
│   │   └── serial_box_blur.c           # Serial CPU implementation
│   ├── mpi/
│   │   └── mpi_box_blur.c              # MPI distributed implementation
│   ├── openmp/
│   │   └── openmp_box_blur.c           # OpenMP multi-threaded implementation
│   ├── opencl/
│   │   └── opencl_box_blur.c           # OpenCL GPU implementation (AMD/NVIDIA/Intel)
│   ├── cuda/
│   │   └── cuda_box_blur.cu            # CUDA GPU implementation (NVIDIA only)
│   └── utils/
│       ├── image_io.c                  # Image I/O functions
│       ├── image_io.h                  # Image I/O header
│       ├── image_io_pgm_old.c          # Legacy PGM format support
│       ├── timer.h                     # Timing utilities
│       ├── generate_test_image.c       # Test image generator
│       └── convert_to_bmp.c            # Image format converter
├── include/
│   ├── box_blur.h                      # Common definitions and prototypes
│   ├── stb_image.h                     # STB image loading library
│   └── stb_image_write.h               # STB image writing library
├── data/
│   └── sample_images/                  # Test images (JPG/PNG/BMP)
├── results/
│   ├── benchmark_results.csv           # Performance metrics CSV
│   ├── performance_graph.png           # Visualization graphs
│   ├── benchmark_outputs/              # Detailed benchmark logs
│   └── output_images/                  # Processed images
├── scripts/
│   ├── compile_all.sh                  # Compile all implementations
│   ├── run_benchmarks.sh               # Run performance benchmarks
│   ├── analyze_results.py              # Statistical analysis
│   ├── compare_results.py              # Compare implementations
│   └── plot_results.py                 # Generate performance graphs
├── docs/
│   ├── OPENCL_SETUP.md                 # OpenCL installation guide
│   ├── QUICK_START.md                  # Quick start guide
│   └── VIVA_GUIDE.md                   # Viva preparation guide
├── benchmark.sh                        # Main benchmark script (Linux)
├── benchmark.bat                       # Main benchmark script (Windows)
├── Makefile                            # Build instructions
└── README.md                           # This file
```

## Prerequisites

### Required (for basic implementations)
- GCC compiler
- Make

### Optional (for parallel implementations)
- **MPI**: OpenMPI or MPICH
- **OpenMP**: GCC with OpenMP support (usually included)
- **OpenCL**: AMD GPU drivers + OpenCL SDK (see `OPENCL_SETUP.md`)
- **CUDA**: NVIDIA CUDA Toolkit + NVIDIA GPU (optional)

## Setup Instructions

### 1. Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install gcc make
sudo apt install openmpi-bin libopenmpi-dev  # For MPI
sudo apt install opencl-headers ocl-icd-opencl-dev mesa-opencl-icd  # For OpenCL
```

**Windows (MSYS2/MinGW):**
```bash
pacman -S gcc make
pacman -S mingw-w64-x86_64-msmpi  # For MPI
# For OpenCL: See OPENCL_SETUP.md
```

### 2. Compile All Implementations

Using Makefile (recommended):
```bash
cd box-blur
make all
```

Or use the shell script:
```bash
chmod +x scripts/compile_all.sh
./scripts/compile_all.sh
```

The compilation will create executables: `serial_box_blur`, `openmp_box_blur`, `mpi_box_blur`, `opencl_box_blur`

### 3. Run Benchmarks

```bash
./benchmark.sh data/sample_images/your_image.jpg
```

Or run individual implementations:
```bash
./serial_box_blur input.jpg output.jpg
./openmp_box_blur input.jpg output.jpg 4  # 4 threads
mpirun -np 4 ./mpi_box_blur input.jpg output.jpg  # 4 processes
```

### 4. View Results

Performance results are automatically saved to:
- `results/benchmark_results.csv` - Raw timing data
- `results/performance_graph.png` - Visual comparison
- `results/output_images/` - Processed images

## Quick Test

Test with the included input image:

```bash
# Compile first
make all

# Run serial version
./serial_box_blur data/sample_images/landscape_512x512.bmp results/output_images/test.jpg

# Run OpenMP with 4 threads
./openmp_box_blur data/sample_images/landscape_512x512.bmp results/output_images/test_omp.jpg 4

# Run full benchmark
./benchmark.sh data/sample_images/landscape_512x512.bmp
```

## Usage Guidelines

- **Image Formats**: Supports JPG, PNG, and BMP via stb_image library
- **Test Images**: Place test images in `data/sample_images/`
- **Results**: Performance metrics stored in `results/benchmark_results.csv`
- **Documentation**: See `docs/QUICK_START.md` for detailed guide
- **Viva Preparation**: See `docs/VIVA_GUIDE.md` for presentation tips
- **GPU Support**: 
  - Use **OpenCL** for AMD Radeon GPUs (see `docs/OPENCL_SETUP.md`)
  - Use **CUDA** only for NVIDIA GPUs

## Performance Comparison

### Expected Speedup (relative to serial, on typical hardware)

| Implementation | Hardware        | Expected Speedup | Best For                    |
|---------------|-----------------|------------------|-----------------------------|
| Serial        | 1 CPU core      | 1x (baseline)    | Small images, debugging     |
| OpenMP        | 4 CPU cores     | 3-4x             | Medium images, CPU-only     |
| MPI           | 4 processes     | 3-4x             | Distributed systems         |
| OpenCL        | AMD Radeon 530  | 10-50x           | GPU acceleration (AMD)      |
| CUDA          | NVIDIA GPU      | 15-100x          | GPU acceleration (NVIDIA)   |

*Actual speedup depends on image size, hardware, and overhead. Larger images show better GPU speedup.*

### Performance Factors

- **Image Size**: Larger images (1024x1024+) benefit more from parallelization
- **Kernel Size**: Larger kernels increase computation per pixel
- **Memory Transfer**: GPU implementations include host-device transfer overhead
- **Thread Count**: OpenMP/MPI performance scales with available cores

## Hardware Compatibility

- ✅ **Serial**: Any CPU
- ✅ **OpenMP**: Any multi-core CPU
- ✅ **MPI**: Any system with MPI installed
- ✅ **OpenCL**: AMD Radeon 530, NVIDIA GPUs, Intel GPUs
- ⚠️ **CUDA**: NVIDIA GPUs only (not compatible with AMD Radeon 530)

## License

This project is licensed under the MIT License. See the LICENSE file for more details.