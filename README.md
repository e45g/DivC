# DivC Compiler (Work in Progress)
Disclaimer: This project is currently under active development and is considered a work in progress. Features may be incomplete or subject to change.

DivC is a C-compatible compiler with a few extra features. It is written in C and is designed to be a simple, yet powerful compiler for a C-like language.

## Features
 - **C-like Syntax:** DivC supports a syntax that is very similar to C, making it easy to learn and use for C developers.
 - **Type System:** DivC has a type system that supports various integer types (`i8`, `i16`, `i32`, `i64`, `u8`, `u16`, `u32`, `u64`), `short`, `long`, `unsigned`, and `void`.
 - **Debugging Utilities:** The compiler includes debugging utilities that can print the AST, which is useful for debugging the compiler itself.

## Getting Started
### Prerequisites
To build and run the DivC compiler, you will need to have `gcc` and `make` installed on your system.

### Building
To build the compiler, simply run the `make` command in the root directory of the project:
```sh
make
```
This will generate an executable file named `divc` in the root directory.

## Usage
To use the compiler, you need to provide a source file as a command-line argument. For example, to compile the `test.c` file located in the `test` directory, you would run the following command:
```sh
./divc test/test.c
```

### Roadmap
 - [x] Support functions
 - [x] Semantic analysis (basic)
 - [x] IR
 - [x] Basic Code generation
 - [ ] Support more features
 - [ ] Semantic analysis (advanced)
 - [ ] More features :)
