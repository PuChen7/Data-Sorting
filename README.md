# Data-Sorting

## Synopsis
This program is a multithreaded C server and client to sort a list of CSV files which contains large amount of data.
The program traverses all subdirectories through a user defined root directory. 
It looks for each CSV file and output a sorted version of each file.

## Algorithm
We used core merge sort idea for sorting algorithm which has excellent average case. 
This modified sorting algorithm enables the program to deal with different special cases in CSV file.

## Client / Server
### Client
Client traverses all subdirectories under the directory it is given. For each file it finds, it spawns a thread. 
The thread read the csv file and construct a request and connect to the server.

### Server
Server opens a port and wait for connections. Once connected, it spawns a service thread that reads client request.
If it requests sorting, then it will perform the sort and store the results.

## Running the program
`./sort_client -c type -h host_name -p 12345` -d thisdir/thatdir -o anotherdir

    * -c indicates the sorting type. 
    
    * -h indicates the hostname of the server.
    
    * -p indicates the port number.
    
    * -d indicates the starting directory.The default behavior will search the current directory.
    
    * -o indicates the output directory. The default behavior will be to output in the same directory as the source directory.

The server has only one parameter: ‘-p’, and is required for operation:

`./sorter_server -p 12345`
