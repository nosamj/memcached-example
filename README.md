# memcached-example
## System Requirements
* Windows 7, 8, 8.1, Server 2008 R2, Server 2012
* Visual Studio 2013

## Build Steps
1. Clone https://github.com/ptanniger/memcached-example.git
2. Open the `Memcache.sln` file in Visual Studio 2013
3. Build Solution

## Solution Projects
* MemcacheLib
 * Contains classes and utilities common to both the Client and Server applications.
* MemcacheClient
 * Console application for issuing Set and Get commands to the Memcache Server
* MemcacheServer
 * Console application that listens for incoming TCP connections from Memcache client applications and hosts the cache where data is stored and fetched.
* MemcacheUnitTests
 * Console application that executes a series of unit tests testing a variety of components of the MemcacheLib project.

## MemcacheServer
    Usage: MemcacheServer.exe (listen_port)
    listen_port: listen on a port different than the default of 30000

    Console Commands:
    quit: Terminates the application.
    printcache: Dumps the current cache to the console.
    clearcache: Clears the cache of all entries.
*NOTE:* When starting the server for the first time you may be prompted by the system firewall to allow the MemcacheServer to listen when connected to specific network groups. Make sure you check the checkbox for each network group so that the server will be able to accept incoming connections from other hosts.

**Example:**
    
    MemcacheServer.exe
    
    Successfully bound on port 30000
    Server started!
    
    Supported Commands:
    quit: Terminates the application.
    printcache: Dumps the current cache to the console.
    clearcache: Clears the cache of all entries.
    
    Enter command: printcache
    DataCache (2 entries)
    Key:key1 Flags:1234 DataSize:11
    Key:key2 Flags:5678 DataSize:11
    
    Enter command: clearcache
    
    Enter command: printcache
    DataCache (0 entries)

## MemcacheClient
    Usage: MemcacheClient.exe [server_ip] [server_port]
    server_ip: IP address of the server to connect to.
    server_port: port on which the server is listening.
    
    Supported Commands:
    quit: Terminates the application.
    set: sets the provided value of the provided key on the connected server.
    get: gets the value of the provided key.

**Example:**  

    MemcacheClient.exe 127.0.0.1 30000
    
    Supported Commands:
    quit: Terminates the application.
    set: sets the provided value of the provided key on the connected server.
    get: gets the value of the provided key.
    
    Enter command:set
    Enter Key:key1
    Enter Flags:1234
    Enter Value:test data 1
    Set successful!
    
    Enter command:set
    Enter Key:key2
    Enter Flags:5678
    Enter Value:test data 2
    Set successful!
    
    Enter command:get
    Enter Key:key1
    Flags:1234 Value:test data 1
    
    Enter command:get
    Enter Key:key1
    Flags:1234 Value:test data 1
