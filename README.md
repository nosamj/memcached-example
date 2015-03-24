# memcached-example
## System Requirements
* Windows 7, 8, 8.1
* Visual Studio 2013

## Solution Projects
* MemcacheLib
 * Contains classes and utilities common to both the Client and Server applications.
* MemcacheClient
 * Console application for issuing Set and Get commands to the Memcache Server
* MemcacheServer
 * Console application that listens for incoming TCP connections from Memcache client applications.
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

## MemcacheClient
    Usage: MemcacheClient [server_ip] [server_port]
    server_ip: IP address of the server to connect to.
    server_port: port on which the server is listening.
    
    Supported Commands:
    quit: Terminates the application.
    set: sets the provided value of the provided key on the connected server.
    get: gets the value of the provided key.
