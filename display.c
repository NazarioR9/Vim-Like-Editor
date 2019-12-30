#include "display.h"


void showFile(int size, int fd_file){
	//Affichage du fichier 
	char buff[size];
	int nr;
	nr = read(fd_file, &buff, size);
	write(1, &buff, nr);

}



