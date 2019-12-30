#include "editorManager.h"


int main(int argc, char *argv[]){
	//Initialize Edit
	initEdit();
	//open Edit
	openEdit(argv[1]);
	//Edit loop
	while(1){
		char c = waitKey();
		processKey(c);
	}

	return argc;
}


