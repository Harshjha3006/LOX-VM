clox : chunk.c compiler.c debug.c main.c memory.c scanner.c value.c vm.c object.c
	gcc object.c chunk.c compiler.c debug.c main.c memory.c scanner.c value.c vm.c -o clox