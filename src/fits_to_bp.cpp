#include <cstddef>  //std::size_t
#include <iostream> // std::cout
#include <fitsio.h>

#include <adios2.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    fitsfile *fptr; /* FITS file pointer, defined in fitsio.h */
    char keyname[81], value[81], comment[81];
    int status = 0; /* CFITSIO status value MUST be initialized to zero! */

    int naxis;
    long naxes[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
    float *data;
    int bitpix;

    MPI_Init(&argc, &argv);
    int rank;
    int size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // adios2::ADIOS adios("config.xml", MPI_COMM_WORLD);

    // adios2::IO io = adios.DeclareIO("imstat_adios_writer");
    // adios2::Engine writer = io.Open("casa_cpp.bp", adios2::Mode::Write);

    adios2::fstream out("casa_cpp.bp", adios2::fstream::out,
                        MPI_COMM_WORLD);

    printf("Reading file %s\n", argv[1]);
    fits_open_file(&fptr, argv[1], READONLY, &status);
    if (status)
    {
        fits_report_error(stderr, status);
        fprintf(stderr, "Error: Unable to open file\n");
        // safe_exit(out);
        return 1;
    }

    /*
    Copying header to BP
    Following program will only run for Images with 1 hdu
    */
    int num_hdu;
    fits_get_num_hdus(fptr, &num_hdu, &status);
    if (num_hdu != 1)
    {
        fprintf(stderr, "Error: Image contains more than 1 hdus\n");
        // safe_exit(out);

        return 1;
    }

    int nkeys, ii;
    fits_get_hdrspace(fptr, &nkeys, NULL, &status); /* get # of keywords */
    printf("Number of keys: %d \n\n", nkeys);

    for (ii = 1; ii <= nkeys; ii++)
    {
        fits_read_keyn(fptr, ii, keyname, value, comment, &status);
        printf("%s : %s \n", keyname, value);

        if (fits_get_keyclass(keyname) == TYP_COMM_KEY)
        {
            printf("%s : %s \n", keyname, comment);
            out.write(keyname, static_cast<std::string>(comment));
            continue;
        }

        // adios2::Variable<std::string> varGlobalValueString = io.DefineVariable<std::string>(keyname);
        // writer.Put(keyname, rank);

        // TODO: write as per data type
        out.write(keyname, static_cast<std::string>(value));
    }

    fits_get_img_param(fptr, 9, &bitpix, &naxis, naxes, &status);
    if (status)
    {
        fits_report_error(stderr, status);
        // safe_exit(out);
        return 1;
    }

    // converting 3d to 4d image by adding extra dimension for polarisation axis. This will modify input fits file
    // takes care of dimensions of data, NAXIS variables and BITPIX
    if (naxis == 3)
    {
        long new_axes[4] = {naxes[0], naxes[1], naxes[2], 1};
        fits_resize_img(fptr, bitpix, 4, new_axes, &status);

        printf("\n 3d converted to 4d \n");

        fits_get_img_param(fptr, 9, &bitpix, &naxis, naxes, &status);

        if (status)
        {
            fits_report_error(stderr, status);
            // safe_exit(out);
            return 1;
        }
    }
    else if (naxis != 4)
    {
        fprintf(stderr, "Error: This only works for 3d or 4d images");
        // safe_exit(out);
        return 1;
    }
    // swapping polarisation and frequency axis. This will modify input fits file
    // 4th dimension (polarisation) will become 1
    else if (naxes[2] == 1 && naxes[3] != 1)
    {
        long new_axes[4] = {naxes[0], naxes[1], naxes[3], naxes[2]};
        fits_resize_img(fptr, bitpix, 4, new_axes, &status);

        printf("\n Axis swwapped \n");

        fits_get_img_param(fptr, 9, &bitpix, &naxis, naxes, &status);

        for (int i = 0; i < naxis; i++)
        {
            printf("NAXIS %d = %ld \n", i, naxes[i]);
        }

        if (status)
        {
            fits_report_error(stderr, status);
            // safe_exit(out);
            return 1;
        }
    }

    if (bitpix != FLOAT_IMG)
    {
        fprintf(stderr, "Error: Input has to be a float type image\n");
        abort();
    }


    // Read offsets from infile
    // long long headaddr;
    // long long databegin;
    // long long dataend;

    // fits_get_hduaddrll(fptr, &headaddr, &databegin, &dataend, &status);
    // if (status)
    // {
    //     fprintf(stderr, "Error: Unable to offset information\n");
    //     fits_report_error(stderr, status);
    //     abort();
    // }
    // printf("Data Start Offset: %lld\n", databegin);

    // Read datat from infile
    // long fpixel[4] = {1, 1, 1, 1};

    // size_t total_pix = naxes[0] * naxes[1] * naxes[2] * naxes[3];
    // data = new float[total_pix];
    // if (data == NULL)
    // {
    //     fprintf(stderr, "Error: Unable to allocate data\n");
    //     return EXIT_FAILURE;
    // }

    // fits_read_pix(fptr, TFLOAT, fpixel, total_pix, 0, data, 0, &status);
    // if (status)
    // {
    //     fprintf(stderr, "Error: Unable to read data\n");
    //     fits_report_error(stderr, status);
    //     return status;
    // }

    // TODO: convert big endian to little endian and store to bp file (per channel using MPI)

    // writer.Close();
    out.close();

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