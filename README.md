# Simple FTP

It includes custom client and server programs. Now only the following commands are supported.

```
list <path>     - retrieve list of files in the specified directory
get <filename>  - get the specified file
```

### compile and run
1. server

```
$ cd server/
$ make
$ ./server
```

2. client

```
$ cd client/
$ make
$ ./client
```