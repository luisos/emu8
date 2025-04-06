# emu8
Intel 8080 CPU emulator
```
emu8 [-h] [-d] [-s] <binfile>
	-h show help
	-s show statistics
	-d debug mode
```

### I/O ports
- #0:  get console status (0 - no input, 1 - there is an input byte)
- #1:  console input/output
