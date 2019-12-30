#ifndef __EDITOR_MANAGER__
#define __EDITOR_MANAGER__

//*********Includes
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h> 

//**********Defines

//********Functions
void addRow(char str[]);
void changeMode();
void closeFile(int fd_file);
void deleteBackward();
void disableCanonique();
void display(int offset);
void enableCanonique();
void getTerminalSize(int *r, int *c);
void initEdit();
void makeRows();
void moveCursor(int ascii);
void normalMode();
void openEdit(char *filename);
void processKey(char c);
void quit();
void showFile(int offset);
void reopenFdBuffer();
void updateCursorPos(int add);
void updateScreen(); //
void setCursorFdBuffer();
void writeDebug(char s[]);

bool isInsert();

int initDebug(char debugFile[]);
int getBufferPos();
int openFile(char filename[], bool inCurrentDir, int right);
int parse_line(char *s, char **argv[]);
int updateBuffer();


char waitKey();

//*******STRUCTS AND ENUMS

struct row{
	char *row;
	int size;
};

typedef struct config{
	int numRow; // numb. of rows (that fits in the editor's screen)
	int numCol; // number of colomns in our editor
	int termX;
	int termY;
	int miceX;
	int miceY;
	int curX; // current x
	int curY; // current y
	int offset; //equal to offset. correspond to offset-line in fd_buffer
	int stop;// cursor position in fd_buffer
	int xmax; // last line x
	int ymax; // last line y
	int fd_debug;
	int fd_buffer;
	int fd_mice;
	int maxChar; // number of character in our editor
	int maxRow; //max rows
	int r; //have sometthing to do with maxRow
	char *filename;
	char *message;
	char buffer[BUFSIZ];
	char buffer1[BUFSIZ];
	bool mode;
	bool eof;
	bool visual;
	struct termios stdin;
	struct termios stdout;
	//flexible array members, are always at the end of a struct
	FILE* file;
} config;


typedef enum ARROW{
	UP = 65, // <ESC>[A  which is the ANSI code
	DOWN, // <ESC>[B
	RIGHT, // <ESC>[C
	LEFT, // <ESC>[D
	NONE = -1 //Nothing | but will be useful in display
} ARROW;

typedef struct action{
	ARROW direction;
	int x;
} action;


//*********GLOBAL VARIABLES
config E;
action A;

#endif
/* if (cmd(keyboard.first)= Crtl){ 
	if (cmd(keybord.next)=a)
	// selectionne tous le texte
 }*/