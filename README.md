# Operating-Systems-Project
The project simulates a secret cyber attack performed by malware.
I've implemented DLL injection into the target process, leveraging Windows API functions to seamlessly inject custom code.
I further extended the project by performing Import Address Table (IAT) hooking, enabling interception and redirection of specific function calls within the target process.

To facilitate communication, I integrated socket programming, enabling the injected code to establish a connection with a remote server.
Upon successful hooking, the project initiated a socket connection, transmitting relevant data to the server. 
