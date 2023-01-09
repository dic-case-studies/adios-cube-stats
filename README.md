# ADIOS - FITS Case Study

## Goal

To use MPI with ADIOS to calculate stats of a FITS datacube, channel by channel. Perform benchmarks to compare this approach to golden approach.

## How to run?

Requirements: conda (miniconda or anaconda) brew

### For python files

1. Run command

```py
conda env create --file requirements.yml
```

This will create a new conda environment with required dependencies.

1. Activate enviroment

```py
conda activate adios_cube_stats
```

1. Run python files using

```py
python <file-name> <arguments>
```

NOTE: On m1 macs, adios2 installed using conda, is of "no-mpi" version. We don't know if adios2 with-mpi is available to install using conda.
This, the python scripts in this repo do not support mpi.

### For c/cpp files

1. Install dependencies:

```sh
brew install open-mpi adios2 cfitsio
```

1. Build source files:

```sh
make all
```

# TODO