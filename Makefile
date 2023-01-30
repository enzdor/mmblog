DEST = /usr/local/bin/mmblog

mmblog: main.c
	gcc main.c -o ${DEST} 

clean:
	rm -f ${DEST}
