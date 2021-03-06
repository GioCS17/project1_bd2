# Source Code


Static Hashing and B+Tree on disk implementation using C++. The documentation of the following code can be generated using Doxygen just executing the following code:

```
doxygen Doxyfile
```

Then go to the latex folder created and execute `make` to get the PDF output.

----

## Requirements

The basic requirements for this example is a conda enviroment:

### Installation on LINUX/UNIX Systems

Download miniconda from https://docs.conda.io/en/latest/miniconda.html

```
chmod +x Miniconda3-latest-Linux-x86_64.sh
bash Miniconda3-latest-Linux-x86_64.sh
source activate base
```

### Installation the following packages

```
conda install -c anaconda cmake
conda install -c conda-forge gtest
conda install -c conda-forge gmock
conda install -c hi2p-perim fmt
```

Note for osx:
` brew install fmt`

-------------
###Build process

```
./build.sh
```

run gtest:
```
./btree-gtest
```

or

```
cd /my_project_path/
mkdir build
cd build
cmake ..
make all
```
