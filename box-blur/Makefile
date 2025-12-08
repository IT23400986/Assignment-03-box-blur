CC = gcc
MPICC = mpicc
NVCC = nvcc

CFLAGS = -O2 -Wall -Iinclude -Isrc/utils
MPIFLAGS = -O2 -Wall -Iinclude -Isrc/utils
OMPFLAGS = -O2 -Wall -Iinclude -Isrc/utils -fopenmp
CUDAFLAGS = -O2 -Iinclude -Isrc/utils
LDFLAGS = -lm

SRC_SERIAL = src/serial/serial_box_blur.c
SRC_MPI = src/mpi/mpi_box_blur.c
SRC_OPENMP = src/openmp/openmp_box_blur.c
SRC_OPENCL = src/opencl/opencl_box_blur.c
SRC_CUDA = src/cuda/cuda_box_blur.cu
SRC_UTILS = src/utils/image_io.c

TARGET_SERIAL = serial_box_blur
TARGET_MPI = mpi_box_blur
TARGET_OPENMP = openmp_box_blur
TARGET_OPENCL = opencl_box_blur
TARGET_CUDA = cuda_box_blur
TARGET_GENERATOR = generate_test_images
TARGET_CONVERTER = convert_to_bmp

.PHONY: all clean serial mpi openmp opencl cuda generator converter

all: serial mpi openmp opencl generator converter

serial: $(TARGET_SERIAL)

mpi: $(TARGET_MPI)

openmp: $(TARGET_OPENMP)

opencl: $(TARGET_OPENCL)

cuda: $(TARGET_CUDA)

generator: $(TARGET_GENERATOR)

$(TARGET_SERIAL): $(SRC_SERIAL) $(SRC_UTILS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_MPI): $(SRC_MPI) $(SRC_UTILS)
	$(MPICC) $(MPIFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_OPENMP): $(SRC_OPENMP) $(SRC_UTILS)
	$(CC) $(OMPFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_OPENCL): $(SRC_OPENCL) $(SRC_UTILS)
	$(CC) $(CFLAGS) -o $@ $^ -lOpenCL $(LDFLAGS)

$(TARGET_CUDA): $(SRC_CUDA) $(SRC_UTILS)
	$(NVCC) $(CUDAFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_GENERATOR): src/utils/generate_test_image.c $(SRC_UTILS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

$(TARGET_CONVERTER): src/utils/convert_to_bmp.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET_SERIAL) $(TARGET_MPI) $(TARGET_OPENMP) $(TARGET_OPENCL) $(TARGET_CUDA) $(TARGET_GENERATOR) $(TARGET_CONVERTER) *.o