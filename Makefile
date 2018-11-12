all:
	gcc main.c readin.c -g -ggdb -o ttake

clean:
	cp lala.bak lala.csv
	rm ttake

