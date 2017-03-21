# chitterchatter
An example of a chat server in C.

This was a project which just so happened to turn into a testing field for my library.
It was to develop with mnl and works quite well. There are a few valgrind/helgrind errors
but I'll write that down to me refactoring without thinking quite a few times!

## Compilation
Simply run make.

## Run
````bash
	./server.out --port 8080
````

And for the client
````bash
	./client.out --port 8080 --host 127.0.0.1 --nick RagingGrim
````

I added some ddos code for the fun.
Inside of the client type:
````
		/ddos <ip> <port> <seconds>
````
