#include <cstddef>   //std::size_t
#include <iostream>  // std::cout
#include <fitsio.h>

#include <adios2.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank;
    int size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::cout << rank << "  " << size << std::endl;
    adios2::ADIOS adios("config.xml", MPI_COMM_WORLD);

    int naxis;
    int status = 0;
    long naxes[4];
    float *data;
    int bitpix;

    fitsfile *fptr;
    printf("Reading file %s\n", argv[1]);
    fits_open_file(&fptr, argv[1], READONLY, &status);

    if (status)
    {
        fits_report_error(stderr, status);
        fprintf(stderr, "Error: Unable to open file\n");
        return status;
    }

    fits_get_img_param(fptr, 4, &bitpix, &naxis, naxes, &status);
    if (status)
    {
        fits_report_error(stderr, status);
        abort();
    }

    if (naxis == 3)
    {
        naxes[3] = 1;
    }

    if (naxes[2] == 1 && naxes[3] != 1)
    {
        naxes[2] = naxes[3];
        naxes[3] = 1;
    }

    if (bitpix != FLOAT_IMG)
    {
        fprintf(stderr, "Error: Input has to be a float type image\n");
        abort();
    }

    // TODO: copy header from fits to bp

    // Read offsets from infile
    long long headaddr;
    long long databegin;
    long long dataend;

    fits_get_hduaddrll(fptr, &headaddr, &databegin, &dataend, &status);
    if (status)
    {
        fprintf(stderr, "Error: Unable to offset information\n");
        fits_report_error(stderr, status);
        abort();
    }
    printf("Data Start Offset: %lld\n", databegin);

    // Read datat from infile
    long fpixel[4] = {1, 1, 1, 1};

    size_t total_pix = naxes[0] * naxes[1] * naxes[2] * naxes[3];
    data = new float[total_pix];
    if (data == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate data\n");
        return EXIT_FAILURE;
    }

    fits_read_pix(fptr, TFLOAT, fpixel, total_pix, 0, data, 0, &status);
    if (status)
    {
        fprintf(stderr, "Error: Unable to read data\n");
        fits_report_error(stderr, status);
        return status;
    }

    // TODO: convert big endian to little endian and store to bp file (per channel using MPI)

    MPI_Finalize();

    return 0;
}

inline int is_little_endian(void)
{
    const unsigned int n = 1U;
    return *((unsigned char *)&n) == 1U;
}

inline void data_swap_byte_order_float(float *raw, size_t size)
{
    if (is_little_endian())
    {
        printf("Little endian conversion\n");
        // TODO: Use reinterpret_cast
        uint32_t *ptr = (uint32_t *)raw;

#pragma omp parallel for schedule(static)
        for (size_t i = 0; i < size; i += 1)
        {
            ptr[i] = __builtin_bswap32(ptr[i]);
        }
    }

    return;
}