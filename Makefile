clox : table.c chunk.c compiler.c debug.c main.c memory.c scanner.c value.c vm.c object.c
	gcc table.c object.c chunk.c compiler.c debug.c main.c memory.c scanner.c value.c vm.c -o clox

debug : table.c chunk.c compiler.c debug.c main.c memory.c scanner.c value.c vm.c object.c
	gcc -g table.c object.c chunk.c compiler.c debug.c main.c memory.c scanner.c value.c vm.c -o clox