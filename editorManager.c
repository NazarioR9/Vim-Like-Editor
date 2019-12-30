#include "editorManager.h"

#define SPECIAL_KEY(c) ((c) & 0x3A) // :c

struct row **cfrow;

//*****************
void initEdit(){
	getTerminalSize( (&E.numRow), &(E.numCol) );
	E.fd_debug = initDebug("debug.txt");
	E.fd_buffer = openFile("buffer.txt", true, 2);
	char s[50];
	sprintf(s, "numRow : %d numCol %d", E.numRow, E.numCol);
	writeDebug( s );
	E.file = fdopen(E.fd_buffer,"r");
	E.offset = 0;
	E.stop = 0;	
	E.eof = false;
	E.mode = true;
	E.visual = false;
	E.filename = NULL;
	cfrow = (struct row **) malloc(sizeof(struct row **));
	E.maxRow = 0;
	E.termX = E.termY = 1;
	disableCanonique();
}

void openEdit(char *filename){
	if(filename != NULL){
		int fd = openFile(filename, false, 0);
		char buff[BUFSIZ];
		int nr = read(fd, &buff, BUFSIZ);
		write(E.fd_buffer, &buff, nr);
		E.filename = malloc(sizeof(char)*E.numCol);
		strcpy(E.filename, filename);
		writeDebug("showfile - filename");
		showFile(0);
		closeFile(fd);
		read(E.fd_buffer, &E.buffer, BUFSIZ);

	}else{
		writeDebug("showfile - NULL");
		//showFile(len, E.fd_buffer);
		strcpy(E.buffer,"");
		//clear the screen
		write(STDOUT_FILENO, "\x1b[2J", 4); //clear all (bc of 2) the screen
		write(STDOUT_FILENO, "\x1b[3J", 4); //reset scroll
		write(STDOUT_FILENO, "\x1b[H", 3); //set the cursor to home position
	}

	E.maxChar = updateBuffer();
	E.curX = E.curY = 0;
	E.termX = E.termY = 1;
	write(STDOUT_FILENO, "\x1b[1;1H", 6);
}

void showFile(int offset){
	//clear the screen
	write(STDOUT_FILENO, "\x1b[2J", 4); //clear all (bc of 2) the screen
	write(STDOUT_FILENO, "\x1b[3J", 4); //reset scroll
	write(STDOUT_FILENO, "\x1b[H", 3); //set the cursor to home position
	if(E.maxChar > 0){
		//
		if(offset < 0)
			offset = 0;
		//
		for(int i = E.maxRow-1; i >= 0; i--)
			free(cfrow[i]);
		
		E.maxRow = 0;
		E.offset = offset;
		//show the rows
		makeRows();
		//Affichage du fichier dans le terminal
		int limit = offset+E.numRow-1;
		for(int i = offset; i < limit && i < E.maxRow; i++){
			//To set the term-cursor at pos (line,1)
			char str[10];
			sprintf(str, "\x1b[%d;1H", (i-offset+1));
			write(STDOUT_FILENO, str, strlen(str));
			//...................
			write(STDOUT_FILENO, cfrow[i]->row, strlen(cfrow[i]->row));
		}

		E.eof = false;
		if(limit >= E.maxRow)
			E.eof = true;


		char str[10];
		sprintf(str, "\x1b[%d;%dH", E.termX, E.termY);
		write(STDOUT_FILENO, str, strlen(str));
	}
}

void makeRows(){
	fseek(E.file, E.offset, SEEK_SET);

	char str[E.numCol];
	char *len;
	while((len = fgets(str, E.numCol, E.file)) != NULL){
		writeDebug("in makeRows fgets");
		writeDebug(str);
		writeDebug("after 'string'");
		str[strlen(len)] = '\0';
		addRow(str);
		//free(str);
	}
	writeDebug("End makerows");
}

void addRow(char str[]){
	int i = E.maxRow;
	void *p1 = (struct row*) malloc(sizeof(struct row *));
	void *p = malloc(strlen(str)+1);

	if(p == NULL){
		perror(" OUPS !!!! P\n");
		exit(0);
	}

	if(p1 == NULL){
		perror(" OUPS !!!! P1\n");
		exit(0);
	}
	
	cfrow[i] = p1;
	cfrow[i]->row = p;
	strcpy(cfrow[i]->row,str);

	cfrow[i]->size = strlen(str);
	/* 
	int j = 0;
	while(str[j] != '\0'){
		if(str[j] == 9){// == TAB
			cfrow[i]->size += 7;
		}
		j++;
	}
	*/
	E.maxRow++;
}

bool isInsert(){
	return E.mode;
}


void changeMode(){
	E.mode = !(E.mode);
}

void quit(){
	//free all mallocs
	for(int i = E.maxRow-1; i >= 0; i--)
		free(cfrow[i]);
	//
	write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
	//close all fds
	fclose(E.file);
	closeFile(E.fd_buffer);
	closeFile(E.fd_debug);
	//
	enableCanonique();
    exit(0);
}

char waitKey(){
	char c;
	while(read(STDIN_FILENO, &c, 1) != 1);
	return c;
}

int parse_line(char *s, char **argv[]){

	if(strlen(s) == 0){
		return 0;
	} 

	unsigned int i = 0;//index on s
	int last_pos = 0; //start of the new word
	int j = 0; //incrementing number of words in argv

	char prec = s[0]; //character at pos (i-1) if 'i' is the current index in s

	while((s[i] != '\0')){

		if(i > 0){
			if((prec == ' ') && (s[i] != prec)){
				last_pos = i;//update last_pos
			}
		}
		

		if( ( (s[i] == ' ') && (prec != s[i]) ) || ( (i == strlen(s)-1) && (s[i] != ' ') ) ){
			char *temp = malloc( (i-last_pos+1)*sizeof(char) );
			int n = last_pos;
			for(unsigned int k = 0; k < i-last_pos+1; k++){
				//printf("%c", s[n]);
				if((k == i-last_pos) && s[n] == ' '){
					n++;
					continue;
				}
				temp[k] = s[n];
				n++;
			}
			temp[i-last_pos+1] = '\0';

			argv[j] = (char **) malloc( strlen(temp) +1);
			*(argv[j]) = temp;
			
			j++;
		}

		prec = s[i];//store the previous character
		i++;//go to the next character index
	}

	argv[j] = NULL;

	return j;
}


void normalMode(){
	changeMode();
	enableCanonique();

	char normal[E.numCol];
	//for visual purpose
	dprintf(STDOUT_FILENO,"\x1b[%d;1H",E.numRow);
	write(STDOUT_FILENO, " ", 1);
	dprintf(STDOUT_FILENO,"\x1b[%d;1H",E.numRow);

	while( fgets(normal, E.numCol, stdin) && (normal[0] != 'i')){
		//************
		sleep(1);
		dprintf(STDOUT_FILENO,"\x1b[%d;1H",E.numRow-1);
		write(STDOUT_FILENO,"\x1b[K",3);
		dprintf(STDOUT_FILENO,"\x1b[%d;1H",E.numRow);
		write(STDOUT_FILENO,"\x1b[K",3);
		//***************
		normal[strlen(normal)-1] = '\0';
		//***************
		char ***tab = malloc(sizeof(char ***));
		int size = parse_line(normal, tab);
		printf("START\n");
		int k = 0;
		while(tab[k]!= NULL){
			printf("'%s'\n", *(tab[k]));
			k++;
		}
		printf("END\n");
		sleep(1);
		if(size ==1){
			if(strcmp(*(tab[0]),":q") == 0){	
				quit();
			}else if(strcmp(*(tab[0]),":w") == 0){
				if(E.filename != NULL){
					int fd = open(E.filename, O_WRONLY | O_CREAT | O_NONBLOCK);
					char buff[BUFSIZ];
					lseek(E.fd_buffer, 0, SEEK_SET);
					int rd = read(E.fd_buffer, &buff, BUFSIZ);
					write(fd, &buff, rd);
					close(fd);
					E.message = "File saved !!!"; 
				}else{
					E.message = "Filename required"; 
				}
			}else{
				write(STDOUT_FILENO, tab[0], strlen(*(tab[0])));
				E.message = "Wrong command";
			}
		}else if(size == 2){
			if(strcmp(*(tab[0]),":w") == 0){
				if(E.filename == NULL){
					E.filename = malloc(strlen(*(tab[1])));
				}
				strcpy(E.filename,*(tab[1]));
				int fd = open(E.filename, O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK);
				if(fd == -1){
					E.message = "Can't access ";
					strcat(E.message, E.filename);
				}else{
					char buff[BUFSIZ];
					lseek(E.fd_buffer, 0, SEEK_SET);
					int rd = read(E.fd_buffer, &buff, BUFSIZ);
					write(fd, &buff, rd);
					close(fd);
					E.message = "File saved !!!"; 
				}
			}
			dprintf(STDOUT_FILENO,"\x1b[%d;1H",E.numRow);
			write(STDOUT_FILENO,"\x1b[K",3);
		}else{
			E.message = "Wrong command";
		}		

		if(strlen(E.message) != 0){
			write(STDOUT_FILENO, E.message, strlen(E.message));
			sleep(1);
		}
		dprintf(STDOUT_FILENO,"\x1b[%d;1H",E.numRow);
		write(STDOUT_FILENO,"\x1b[K",3);
		dprintf(STDOUT_FILENO,"\x1b[%d;1H",E.numRow);

	}
	disableCanonique();
	/*
	char c;
	while( (c = waitKey()) != 'i'){
		if( SPECIAL_KEY(c) ){
			if(c == 'q'){
				quit();
			}
		}
	}
	*/
	changeMode();
}

int updateBuffer(){
    //Seek the cursor to the begining 
    lseek(E.fd_buffer, 0, SEEK_SET);
    //Now, we can read
    char *buff[BUFSIZ];
    return read(E.fd_buffer, buff, BUFSIZ);
}

int getBufferPos(){
	int pos = 0;
	for(int i = 0; i < (E.offset + E.termX -1) && i < E.maxRow; i++){ 
		pos += cfrow[i]->size;
	}
	pos += (E.termY-1);

	return pos;
}

void updateCursorPos(int add){

	/***********************
	*	FILE CURSOR
	*/



	/***********************
	*	TERMINAL CURSOR
	*/
	
	/*termY and curY are the same since ymax is fixed
	*but for visual purpose, we taking modulo(E.numCol+1)
	*instead of modulo(E.numCol)
	*/

	E.termY += add;

	if(E.termY < 1){ 
		if(E.termX <= 1){
			E.termY = 1;
		}else{
			E.termX--;
			E.termY = cfrow[E.offset + E.termX-1]->size;
		}
	}
	/*
	if(E.xmax < E.numRow-1){
		E.termX += (E.termY/ (E.numCol+1) );
	}
	*/

	if(E.termY == E.numCol)
		E.visual = true;

	E.termX += (E.termY / E.numCol);
	E.termY %= (E.numCol);

	/*
	if(E.curX < E.numRow)
		E.termX = E.curX;
	*/

	//**********************
	//means that we're beyond the cursor grid
	if(E.termX < 1) 
		E.termX = 1;

	if(E.termY < 1) 
		E.termY = 1;

	if( E.termX > (E.numRow-1) )
		E.termX = E.numRow-1;

	if( E.termX > (E.maxRow+1) && E.maxRow != 0){
		writeDebug("term ---> maxRow");
		E.termX = E.maxRow+1;
	}
	//**********************

}


void deleteBackward(){
	E.stop = getBufferPos() -1;
	//let read in store fd_buffer in buffer
	lseek(E.fd_buffer, 0, SEEK_SET);
	//let's delete the last character
	int n = 0;
	if(E.stop == E.maxChar-1){
		if(E.stop > -1){
			//read all the characters from the beginig to the char to delete
			n = read(E.fd_buffer, &E.buffer, E.stop);
			//delete everything in fd_buffer
			reopenFdBuffer();
			//write in fd_buffer
			write(E.fd_buffer, &E.buffer, n);
		}
	}else if(E.stop > 0){
		//read all the characters from the beginig to the char to delete
		n = read(E.fd_buffer, &E.buffer, E.stop);
		//We set the cursor after the character to delete
		char c;
		read(E.fd_buffer, &c, 1);
		//add the rest in the buffer
		n += read(E.fd_buffer, &E.buffer1, BUFSIZ);
		reopenFdBuffer();
		//writing without the deleted character
		write(E.fd_buffer, &E.buffer, E.stop);
		write(E.fd_buffer, &E.buffer1, n-E.stop);
	}else{
		n = read(E.fd_buffer, &E.buffer, BUFSIZ);
		//we truncate fd_buffer
		reopenFdBuffer();
		//writing without the deleted character
		write(E.fd_buffer, &E.buffer, BUFSIZ);
	}
	//reduce the y-pos and re-ajust the position
	updateCursorPos(-1);
}


//to delete
void reopenFdBuffer(){
	close(E.fd_buffer);
	E.fd_buffer = openFile("buffer.txt", true, 2);
}

void writeDebug(char s[]){
	char s1[100] = "";
	strcat(s1, s);
	strcat(s1, " \n");
	write(E.fd_debug, &s1, strlen(s1));
}

void processKey(char c){
	writeDebug("processKey....");
	E.stop = getBufferPos();

	if( c == 27 && isInsert() ){ //if insert and echap
		char nc;
		if((read(STDIN_FILENO, &nc, 1) == 1) && (nc == '[')){ //ask if nc == '['
			read(STDIN_FILENO, &nc, 1); //thus, here nc might be A, B, C or D
			writeDebug("Moving the cursor");
			moveCursor((int) nc);
			display(E.offset);
		}else{
			normalMode();
			updateScreen();
		}
	}else{
		if(c == 127){
			writeDebug("Deleting a character");
			deleteBackward();
			E.maxChar--;
			if(E.maxChar < 0)
				E.maxChar = 0;
		}else{

			if( (c >= 32 && c < 127) || (c >= 10 && c <= 15) ){
				writeDebug("Now writing");

				if((E.stop+1) < E.maxChar){//to insert some char
					lseek(E.fd_buffer, 0, SEEK_SET);
					read(E.fd_buffer, &E.buffer, E.stop);
					read(E.fd_buffer, &E.buffer1, BUFSIZ);
					reopenFdBuffer();
					write(E.fd_buffer, &E.buffer, E.stop);
				}
				
				if(c == 10 || c == 13){//newLine key
					writeDebug("Entrée ou retour chariot");
					char str0[50] = "";
					sprintf(str0,"writing : [%d;%dH",E.termX,E.termY);
					writeDebug(str0);
					E.termX += 1;
					E.termY = 1;
					char str[70] = "";
					sprintf(str,"Before updateScreen [%d;%dH",E.termX,E.termY);
					writeDebug(str);
					updateCursorPos(0);
					char str1[70] = "";
					sprintf(str1,"After updateScreen [%d;%dH",E.termX,E.termY);
					writeDebug(str1);
					
					write(E.fd_buffer,"\r\n", 2);
				}else{
					write(E.fd_buffer, &c, 1);
					updateCursorPos(1);
				}

				if(E.stop < E.maxChar){//same for insert char 
					write(E.fd_buffer, &E.buffer1, E.maxChar-E.stop);
				}
				E.maxChar++;
			}
		}
		updateScreen();
	}
	fseek(stdin,0,SEEK_END);
	char str[5] = "";
	sprintf(str,"%d",E.maxChar);
	writeDebug(str);
	writeDebug("End processKey....");
}

void moveCursor(int ascii){ //in cmd
	int val;
	switch(ascii){
		case UP:
			E.termX--;
			if((E.termX >= 1) && (E.termY > cfrow[E.offset + E.termX-1]->size))
				E.termY = cfrow[E.offset + E.termX-1]->size;
			//
			A.direction = UP;
			A.x = E.termX;
			break;
		case DOWN:
			//
			E.termX++;
			val = E.offset + E.termX -1;

			char str1[70] = "";
			sprintf(str1,"val : %d , max : %d\n",val, E.maxRow);
			writeDebug(str1);

			if( val > -1 && val <  E.maxRow){
				if((E.termX >= 1) && (E.termY > cfrow[val]->size))
					E.termY = cfrow[val]->size;
			}
			//
			A.direction = DOWN;
			A.x = E.termX;
			break;
		case RIGHT:
			val = E.offset + E.termX -1;
			if( val > -1 && val <  E.maxRow){
				if(E.termY <= cfrow[val]->size){
					updateCursorPos(1);
				}
			}
			break;
		case LEFT:
			if(E.termY > 1)
				updateCursorPos(-1);
			break;
	}
	updateCursorPos(0); // just to update ymax
	setCursorFdBuffer();

}

void setCursorFdBuffer(){ //in buffer (E.fd_buffer)
	writeDebug("setCursorFdBuffer....");
	E.stop = getBufferPos();
	lseek(E.fd_buffer, E.stop, SEEK_SET);
	char str[10] = "";
	sprintf(str,"\033[%d;%dH",E.termX,E.termY);
	write(STDOUT_FILENO, str,strlen(str));
	writeDebug(str);
    writeDebug("End setCursorFdBuffer....");

}

int openFile(char filename[], bool inCurrentDir, int right){
	int fd_file;
	if(inCurrentDir){
		char dir[256] = "";
		getcwd(dir, sizeof(dir));
		strcat(dir, "/");
		strcat(dir, filename);
		switch(right){
			case 0:
				fd_file = open(dir, O_RDONLY | O_CREAT | O_NONBLOCK, 0773);
				break;
			case 1:
				fd_file = open(dir, O_WRONLY | O_CREAT | O_APPEND | O_NONBLOCK, 0773);
				break;
			case 2:
				fd_file = open(dir, O_RDWR | O_CREAT | O_TRUNC | O_NONBLOCK, 0773);
				break;
		}
	}else{
		switch(right){
			case 0:
				fd_file = open(filename, O_RDONLY | O_CREAT | O_NONBLOCK, 0773);
				break;
			case 1:
				fd_file = open(filename, O_WRONLY | O_CREAT | O_APPEND | O_NONBLOCK, 0773);
				break;
			case 2:
				fd_file = open(filename, O_RDWR | O_CREAT | O_TRUNC | O_NONBLOCK, 0773);
				break;
		}
	}

	return fd_file;
}


void closeFile(int fd_file){
	close(fd_file);
}

int initDebug(char debugFile[]){
	int fd_debug = openFile(debugFile, false, 2);
	if(fd_debug < 0){
		perror("File not charged");
		exit(0);
	}
	//Checkpoint
	write(fd_debug, "..........\n", 11);
	
	return fd_debug;
}


void disableCanonique(){
	//Passage en mode non canonique

	if( tcgetattr(STDIN_FILENO, &E.stdin)  != 0){
		writeDebug("(STDIN_FILENO) Not available");
		exit(0);
	}

	if(E.stdin.c_lflag & ICANON){

		struct termios new = E.stdin;
		cfmakeraw(&new);
		new.c_cc[VMIN] = 1;
		new.c_cc[VTIME] = 0;


		if(tcsetattr(STDIN_FILENO, TCSANOW, &new) != 0){
			writeDebug("Failed to get (STDIN_FILENO) attributs");
			exit(0);
		}

		writeDebug("ICANON mode disabled (STDIN_FILENO)");	
	}

}

void enableCanonique(){

	//Passage en mode canonique
	
	if( tcsetattr(STDIN_FILENO, TCSANOW,  &E.stdin) != 0){
		writeDebug("Failed to enable ICANON mode (STDIN_FILENO)");
		exit(0);
	}

	writeDebug("ICANON mode enabled (STDIN_FILENO AND STDOUT_FILENO) ");
}

void getTerminalSize(int *r, int *c){

	struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    
    *r = w.ws_row;
    *c =  w.ws_col;
}

void updateScreen(){
    //display in Edit what's in fd_buffer
	showFile(E.offset);
    /*just for visual purpose
    *Help fix with the cursor 
    *by stoping it from being at pos (xmax-1, 0)
    *but at (xmax,0)
    if(E.visual){
    	write(STDOUT_FILENO, " ",1);
    	E.visual = false;
    	E.termY++;
    }
    */
    //Seek the cursor to the last position
    setCursorFdBuffer();
    //debug message
    //dprintf(E.fd_debug, "x: %d, xmax: %d y: %d \n", E.curX, E.xmax, E.curY);
    //dprintf(E.fd_debug, "termx: %d, termy: %d \n", E.termX, E.termY);
}

void display(int offset){
	if(A.direction == UP && A.x < 1){
		//E.termX = (int)((E.numRow-1)/2);
		showFile(offset - 1);
	}else if(A.direction == DOWN && A.x >= E.numRow){
		//E.termX = ;
		if(E.eof){
			showFile(offset);
		}else{
			showFile(offset+1);
		}
	}
	char str[10] = "";
	sprintf(str,"off : %d",E.offset);
	writeDebug(str);
	A.direction = NONE;
}