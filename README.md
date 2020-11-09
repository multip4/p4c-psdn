# p4c-multip4
p4c-multip4 converts P4 programs into Xilinx SDNet backends with parallelization.

## How to build
1. Clone [p4c](https://github.com/p4lang/p4c). 
    ```
    git clone --recursive https://github.com/p4lang/p4c.git
    ```
2. Move to the checkpoint. 
    ```
    cd p4c
    git checkout 8eb631f9238416e4f13e2f222116be7a68570137
    ```
3. Make extensions directory. 
   ```
   mkdir extensions
   cd extensions
   ```
4. Clone p4c-psdn. 
    ```
    git clone https://github.com/multip4/p4c-psdn.git
    ```
    Or you can make a simbolic link in the `extensions` directory.
    ```
    ln -s <path/to/project/directory> p4c-multip4
    ```
5. Make build directory and build.
    ```
    cd .. 
    mkdir build
    cmake <optional arguments>
    make -j4
    make -j4 check
    ```
6. Follow further instructions in [p4c README](https://github.com/p4lang/p4c#getting-started).

## Dependencies
Check the dependencies on [p4c README](https://github.com/p4lang/p4c#dependencies).
p4c-multip4 does not require other dependencies that are not required from p4c.

## Development Notes
- Support conversion of headers and a parser.
- Merge DependencyAnalysis project to here: now support data dependency
analysis of tables.

## Contact Info
Seungbin Song (seungbin@yonsei.ac.kr)
