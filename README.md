# Assembler Project Readme

This readme file provides an overview and instructions for the Assembler project developed in C on Ubuntu. The project aims to implement an assembler program that translates assembly language code into machine code.

## Project Structure

The project consists of the following files:

 `firstRead.c`: Contains the implementation of the first pass of the assembler.
 `secondRead.c`: Contains the implementation of the second pass of the assembler.
 `main.c`: The main entry point of the program.
 `utility.c`: Contains utility functions used in the assembler.
 `makefile`: The makefile for compiling and building the project.
6. `assembler.h`: The header file containing function declarations and necessary data structures.

## Prerequisites

To compile and run the assembler project, ensure that the following prerequisites are met:

1. Ubuntu operating system (or any Linux distribution)
2. GCC compiler installed
3. Basic knowledge of the C programming language

## Compiling the Project

Follow these steps to compile the assembler project:

1. Open a terminal window.
2. Navigate to the project directory using the `cd` command.
3. Use the following command to compile the project using the provided `makefile`:

```shell
make
```

4. The `make` command will build the project and generate an executable file named `assembler`.

## Running the Assembler

After successfully compiling the project, execute the following steps to run the assembler:

1. Ensure that the project directory contains the necessary input files or assembly code.
2. In the terminal, execute the following command to run the assembler:

```shell
./assembler [arguments]
```

3. Replace `[arguments]` with any necessary command-line arguments required by your assembler implementation.

## Output

The output of the assembler program will depend on the specific implementation. It may include generated machine code, symbol tables, or any other relevant information. Please refer to the source code and implementation details to understand the specific output of this assembler project.

## Contributing

Contributions to this project are welcome. If you find any issues or have suggestions for improvements, please feel free to submit a pull request or open an issue on the project's GitHub repository.
