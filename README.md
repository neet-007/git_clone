# Git Clone  

## Overview  

This project is a C implementation of [wyag](https://wyag.thb.lt/) that works in a Unix environment.  
The only external libraries used are:  
- **zlib** for compression and decompression  
- **OpenSSL** for hashing  

## Features  

The following Git commands are supported:  

- `init`  
- `object-hash`  
- `cat-file`  
- `log`  
- `ls-tree`  
- `checkout`  
- `show-ref`  
- `tag`  
- `rev-parse`  
- `ls-files`  
- `check-ignore`  
- `status`  
- `rm`  
- `add` (coming soon)  
- `commit` (coming soon)  

## Usage  

To compile the code, simply run:  
```bash
make
```

Ensure you have zlib and OpenSSL installed beforehand.

Run the program with any of the supported commands:

```bash
git_clone COMMAND -[FLAGS] ARGS...
```

## Notes

- The index write functionality does not fully match Git's behavior yet.
- This may cause issues with some commits.
