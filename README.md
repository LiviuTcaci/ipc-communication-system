# Inter-Process Communication System

A C implementation that demonstrates various inter-process communication (IPC) mechanisms including named pipes, shared memory, and memory-mapped files.

## Overview

This project implements a client-server communication system that processes requests through named pipes. It handles shared memory operations, file memory mapping, and reading from a custom SF (Section File) format according to specific rules.

## Features

- Pipe-based communication with a defined protocol
- Shared memory creation and management
- File memory mapping for efficient file operations
- Processing of SF file format with section awareness
- Logical memory space mapping

## Compilation

Compile the program with:

```bash
gcc -Wall a3.c -o a3 -lrt
```

## Running

Run the executable with:

```bash
./a3
```

The program will:
1. Create the response pipe (RESP_PIPE_45862)
2. Open the request pipe (REQ_PIPE_45862)
3. Establish connection with the testing program
4. Process incoming requests

## Implemented Requests

### ECHO
Simple request that returns "ECHO VARIANT 45862"

### CREATE_SHM
Creates a shared memory region with specified size and permissions

### WRITE_TO_SHM
Writes a value at a specific offset in the shared memory region

### MAP_FILE
Memory-maps a file for efficient access

### READ_FROM_FILE_OFFSET
Reads a number of bytes from a specified offset in the memory-mapped file

### READ_FROM_FILE_SECTION
Reads a number of bytes from a specified section in a SF file

### READ_FROM_LOGICAL_SPACE_OFFSET
Reads from the logical memory space of a SF file using alignment rules

### EXIT
Cleans up resources and terminates the program

## SF File Format

The SF (Section File) format consists of:

- A header containing metadata (magic number, version, section count)
- Section headers with name, type, offset and size information
- File body with actual section data

The format supports special rules for section alignment and reading.

## Protocol

Communication follows a defined protocol:
- String fields: prefixed with byte indicating length
- Number fields: raw binary data (unsigned int)
- Request format: `<request_name> [parameters]`
- Response format: `<request_name> <status> [data]`

## Dependencies

- Linux operating system
- C standard library
- RT library (`-lrt`) for shared memory operations
