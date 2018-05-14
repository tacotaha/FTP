# FTP
An implementation of the File Transfer Protocol: https://tools.ietf.org/html/rfc959


                                            -------------
                                            |/---------\|
                                            ||   User  ||    --------
                                            ||Interface|<--->| User |
                                            |\----^----/|    --------
                  ----------                |     |     |
                  |/------\|  FTP Commands  |/----V----\|
                  ||Server|<---------------->|   User  ||
                  ||  PI  ||   FTP Replies  ||    PI   ||
                  |\--^---/|                |\----^----/|
                  |   |    |                |     |     |
      --------    |/--V---\|      Data      |/----V----\|    --------
      | File |<--->|Server|<---------------->|  User   |<--->| File |
      |System|    || DTP  ||   Connection   ||   DTP   ||    |System|
      --------    |\------/|                |\---------/|    --------
                  ----------                -------------

                  Server-FTP                   USER-FTP
## To Build 
 
 Simply clone this repository and use the Makefile to build
 
 ```git clone https://github.com/tazzaoui/FTP.git && cd FTP && make```
 
 You'll end up with two executables, namely ```server``` and ```client```
 
 ## To Run
 First run the server using the ```server ``` executable. The default listening port is 4444 but the -p flag can be used to specify a different port. Make sure your firewall's rules don't conflict with the server's configuration. 
 
 Then connect a client by invoking the ```client``` executable. The -i and -p flags can be used to specify the server's IP address and port number respectively. Note that the client will connect to port 4444 by default, but should be configured to match the server.
 
 You'll then be greeted with a tty, at which point you can enter ```help``` to view the available commands
 
 ## Example
```[machine1@192.168.0.19] $./server -p 4445 #Run server on machine1 listening on port 4445```
```[machine2@192.168.0.20] $./client -i 192.168.0.19 -p 4445 #Connect to port 4445 on machine1```
