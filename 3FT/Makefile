#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3
# Author: Matthew Okechukwu, Rain Huang
#--------------------------------------------------------------------

all: ft

clean: 
	rm -f ft ft.o ft_client.o NodeFT.o path.o dynarray.o


ft: ft.o ft_client.o NodeFT.o path.o dynarray.o
	gcc217 -g ft.o ft_client.o NodeFT.o path.o dynarray.o -o ft

ft.o: ft.c ft.h
	gcc217 -g -c ft.c

NodeFT.o: NodeFT.c NodeFT.h
	gcc217 -g -c NodeFT.c

path.o: path.c path.h
	gcc217 -g -c path.c

dynarray.o: dynarray.c dynarray.h
	gcc217 -g -c dynarray.c

ft_client.o: ft_client.c ft.h a4def.h 
	gcc217 -g -c ft_client.c

