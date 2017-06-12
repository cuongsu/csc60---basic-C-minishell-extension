# Simple Makefile for csc60mshell program
CC=gcc
CFLAGS=-g -Wall
EXE=csc60mshell

all: csc60mshell
mshell: csc60mshell.c
	gcc -o csc60mshell csc60mshell.c
