# Distributed Systems Key-Value Store

This project is a **distributed key-value storage system** implemented in **C**, developed as part of the **Distributed Systems** course at the **University of Lisbon**. The system evolved through four phases, incorporating key distributed systems concepts such as **remote procedure calls (RPC)**, **multi-threading**, **concurrency control**, and **fault tolerance with replication**.

## Features  
- **Key-Value Storage:** Implements a hash table-based key-value store.  
- **Remote Procedure Calls (RPC):** Uses **Protocol Buffers** for efficient serialization.  
- **Multi-Client Support:** Implements a **multi-threaded server** with **one-thread-per-client** concurrency model.  
- **Server Statistics:** Tracks real-time server statistics, including the number of operations and active clients.  
- **Fault Tolerance:** Implements **Chain Replication** with **ZooKeeper** for coordination and failover.  
- **Dynamic Server Management:** New servers can dynamically join and synchronize data.  

## Installation & Usage  

### Requirements  
Ensure you have the following installed on your system:  
- **GCC** (for compiling C programs)  
- **Make** (for managing build automation)  
- **Protocol Buffers (protobuf-c)**  
- **ZooKeeper** (for distributed coordination)  

### Building the Project  
Clone the repository and compile the project:  
```bash
git clone https://github.com/yourusername/distributed-kv-store.git  
cd distributed-kv-store  
make
```
### Running the Server
Start the server with:
```bash
./server_hashtable <port> <num_lists> <zookeeper_ip:port>
```
Example:
```bash
./server_hashtable 5000 10 127.0.0.1:2181
```
### Running the Client
Connect to the key-value store with:
```bash
./client_hashtable <zookeeper_ip:port>
```
Example:
```bash
./client_hashtable 127.0.0.1:2181
```

### Available Commands
- `put <key> <value>` - Insert or update a key-value pair.
- `get <key>` - Retrieve the value associated with a key.
- `del <key>` - Remove a key-value pair.
- `size` - Get the total number of stored key-value pairs.
- `getkeys` - Retrieve all stored keys.
- `gettables` - Retrieve all key-value pairs.
- `stats` - Get real-time server statistics.
- `quit` - Disconnect the client.

## Project Structure
```bash
/kv-storage-service
│── include/          # Header files (.h)
│── source/           # Source code files (.c)
│── lib/              # Compiled libraries
│── object/           # Object files (.o)
│── binary/           # Executable binaries
│── makefile          # Build automation
│── README.md         # Project documentation
```

## Contributors
- Larissa Oliveira
- Nickolas Ficker
- Tiago Lopes
