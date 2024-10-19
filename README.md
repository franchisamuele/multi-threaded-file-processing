## Multi-threaded .dat File Processing

This university project is designed to test knowledge of processes, threads, sockets, and signals in C. The program recursively traverses directories to locate and process large `.dat` files using multiple threads. Each `.dat` file contains a series of integers or decimal numbers, which may be separated by empty lines. The goal is to calculate the arithmetic mean and standard deviation of the numbers in each file.

### Program Structure

1. **Master Process**: Recursively scans directories for `.dat` files and delegates the processing tasks to multiple worker threads.
2. **Worker Threads**: Each worker reads a file, computes the mean and standard deviation, and sends the result to the collector process via a socket.
3. **Collector Process**: Receives results from the worker threads and displays them in a tabular format showing the file name, number of valid numbers, mean, and standard deviation.

### Socket Communication

The worker threads communicate with the collector through an `AF_UNIX` socket. Each thread sends results to the collector, which acts as a server. The communication is handled through an individual connection for each worker, ensuring multiple messages can be sent per connection.

### Output Example

```
n      avg     std      file
--------------------------------------------------
3      3.00    0.81     ./prova1.dat
7     30.71   29.81     ./provadir/provadir3-3.dat
...
```

### Usage and Testing

The project includes a `Makefile` for easy compilation and testing. Upon execution, the `Makefile` should compile the program without any warnings or errors, ensuring compliance with laboratory machine requirements. Additionally, there are three predefined testing targets:

1. `make test1`: Runs the program with 1 worker thread.
2. `make test2`: Runs the program with 5 worker threads.
3. `make test3`: Runs the program with 5 worker threads and uses Valgrind to check for memory leaks and errors.

>Note: For more information, refer to the detailed project specification in the attached PDF.
