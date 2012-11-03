all:
	gcc -g -o ptmalloc *.c -lpthread
	gcc -g -o jemalloc *.c -lpthread -ljemalloc
	gcc -g -o tcmalloc *.c -lpthread -ltcmalloc
	gcc -g -o llmalloc *.c -lpthread -lllalloc

clean:
	rm -rf *.o ptmalloc jemalloc tcmalloc llmalloc 
