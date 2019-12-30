#include "fileManager.h"


int openFile(char filename[], bool inCurrentDir, int right){
	int fd_file;
	if(inCurrentDir){
		char dir[256] = "";
		getcwd(dir, sizeof(dir));
		strcat(dir, "/");
		strcat(dir, filename);
		switch(right){
			case 0:
				fd_file = open(dir, O_RDONLY | O_CREAT | O_NONBLOCK);
			case 1:
				fd_file = open(dir, O_WRONLY | O_CREAT | O_NONBLOCK);
			case 2:
				fd_file = open(dir, O_RDWR | O_CREAT | O_NONBLOCK);
		}
	}else{
		switch(right){
			case 0:
				fd_file = open(dir, O_RDONLY | O_CREAT | O_NONBLOCK);
			case 1:
				fd_file = open(dir, O_WRONLY | O_CREAT | O_NONBLOCK);
			case 2:
				fd_file = open(dir, O_RDWR | O_CREAT | O_NONBLOCK);
		}
	}

	return fd_file;
}


void closeFile(int fd_file){
	close(fd_file);
}

int initDebug(char debugFile[]){
	int fd_debug = openFile(debugFile, false);
	if(fd_debug < 0){
		perror("File not charged");
		exit(0);
	}
	//Checkpoint
	write(fd_debug, "..........", 10);
	
	return fd_debug;
}