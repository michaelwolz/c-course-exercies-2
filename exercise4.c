/* Aufgabe4 ****************************************************************
**
** Author: Michael Wolz
** Matrikelnummer: 1195270
**
** Author: Aaron Winziers
** Matrikelnummer: 1176638
**
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h> 

#define MAX_FILE_LENGTH		10000000000

struct node { //Binary Tree zum sortieren der Wörter (wie in beispiel: http://dblps.uni-trier.de/~ley/kp14/lexikon.c)
	char		*w;
	int	 	count;
	struct node	*l, *r;
};

struct lNode { //Struct für eine Liste mit Elementen welche einen tree als Kind und einen counter haben.
	int 	count;
	char 	*mName;
	struct lNode	*next;
	struct node 	*tree;
} *magazineList;

struct sortNode { //Struct für eine doppelt verkettete Standard-Liste um die Ergebnisse nach Auftreten der Wörter zu sortieren
	struct node *data; 
	struct sortNode *prev, *next;	
};

/************************************************************************
 *
 * 	Loads a record completely into the memory.
 *  (wie in beispiel: http://dblps.uni-trier.de/~ley/kp14/lexikon.c)
 *
 ************************************************************************/

char *read_file(char *file_name) { 
	FILE	*stream;
  	struct stat stat_buf;
  	char	*file_start;
	time_t	mtime;

	if (file_name == NULL)
		return 0;

	if ((stream = fopen(file_name,"rb"))==NULL) {
		return 0;
	}

  	/* get file-size */

  	if (fstat(fileno(stream),&stat_buf)) {
		fclose(stream);
    		return 0;
  	}

  	if (stat_buf.st_size > MAX_FILE_LENGTH) {
		fclose(stream);
		return 0;
	}

  	/* allocate memory, read file */

  	if ((file_start = (char *) malloc(stat_buf.st_size+1))==NULL) {
		fclose(stream);
    		return 0;
  	}
  	if (fread(file_start,stat_buf.st_size,1,stream)!=1) {
    		free(file_start);
		fclose(stream);
    		return 0;
  	}
	fclose(stream);

  	*(file_start+stat_buf.st_size) = '\0';

  	return file_start;
}

/************************************************************************
 *
 *	BINARY TREE 
 *  (wie in beispiel: http://dblps.uni-trier.de/~ley/kp14/lexikon.c)
 *
 ************************************************************************/

//Insert to Binary Tree
void incr(struct node **root, char *w) {
	if (root == NULL || w == NULL) {
		return;
	}
	if (*root == NULL) {
		struct node *n;
		int l = strlen(w);

		n = (struct node *) malloc(sizeof(struct node));
		if (n == NULL)
			return;
		n->w = (char *) malloc(l+1);
		if (n->w == NULL)
			return;
		strcpy(n->w,w);
		n->count = 1;
		n->l = NULL;
		n->r = NULL;

		*root = n;
		return;
	}

	int c = strcmp((*root)->w,w);
	if (c == 0) {
		((*root)->count)++;
		return;
	}
	if (c < 0) 
		incr(&((*root)->r),w);
	else 
		incr(&((*root)->l),w);
}

//wie in beispiel: http://dblps.uni-trier.de/~ley/kp14/lexikon.c)
void words(struct node **root, char *s) {
	int state = 0;
	char c;
	char *start;

	while (c = *s) {
		if (isalpha(c)) {
			if (!state) {
				start = s;
				state = 1;
			}
		} else {
			state = 0;
			if (start) {
				*s = '\0';
				incr(root, start);
				*s = c;
			}
			start = NULL;
		}
		s++;
	}
}

/************************************************************************
 *
 *	LIST MANAGEMENT
 *
 ************************************************************************/
//Algorithmus zum sortierten Einfügen in eine Liste (absteigend nach der Anzahl des jeweiligen Wortes)
void sortedInsert(struct sortNode **list, struct node *data) {
	struct sortNode *l = *list;
	struct sortNode *n;
	n = (struct sortNode *) malloc(sizeof(struct sortNode)); //Speicherplatz für ein neues Liste-Item reservieren
	n->data = data; 
	n->next = NULL;
	n->prev = NULL;

	//Einsortieren an die korrekte Stelle in der Liste 
	if (list == NULL || data == NULL) 
		return;

	if (*list == NULL) {	
		*list = n;
		return;
	} else if (l->data->count < data->count) {
		n->next = l;
		l->prev = n;
		*list = n;
		return;
	} else {	
		while (l->data->count >= data->count && l->next != NULL) {
			l = l->next;
		}	
		if (l->next == NULL) {
			l->next = n;
			n->prev = l;
		} else {	
			n->prev = l->prev;
			n->next = l;
			l->prev = n;
			n->prev->next = n;
		}
	}
}	

//Ausgeben einer sortierten Liste mit einem variablen Limit (für eine schönere Ausgabe)
void itterateList(struct sortNode *list, int limit) {
	struct sortNode *n = list;
	int counter = 0; 
	while (n && counter < limit) { //Solange die sortierte Liste ausgeben, bis das Limit erreicht wurde
		printf("%d %s\n", n->data->count, n->data->w);
		n = n->next;
		counter++;
	}
}
	
//Speicherplatz einer Liste wieder freigeben 
void freeList(struct sortNode* start) {
   struct sortNode* tmp;

   while (start != NULL)
    {
       tmp = start;
       start = start->next;
       free(tmp);
    }
}

//Magazin-Item in ersellen und in die Magazin-Liste einsortieren
void insert(char *mName, char *title) {
	struct lNode *n;
	struct node *tr = NULL;

	if (mName == NULL)
		return;

	n = (struct lNode *) malloc(sizeof(struct lNode));

	if (n == NULL)
		return;
	n->count = 1;
	n->mName = mName;
	n->next = magazineList;	
	words(&n->tree, title);	
	magazineList = n;
}

//Falls das Magazin schon exisistiert wird es nur geupdated und der Artikel-Count erhöht
void update(struct lNode *n, char *title) {
	words(&(n->tree), title);
	n->count++;
}

//Gucken ob ein neues Magazin-Item erstellt werden muss oder lediglich update ausgeführt werden muss
void insertOrUpdate(char *mName, char *title) {
	struct lNode *n = magazineList;
	int found = 0;

	while (n) {
		if (strcmp(n->mName, mName) == 0) {
			update(n, title);
			found = 1;
			break;
		}
		n = n->next;
	}

	if (!found) {
		insert(mName, title);
	}
}

/************************************************************************
 *
 *	traverse Funktion zum durchforsten eines Binary Trees
 *  (wie in beispiel: http://dblps.uni-trier.de/~ley/kp14/lexikon.c)
 *
 ************************************************************************/

void traverse(struct node *r, struct sortNode **list) {
	if (r == NULL) 
		return;
	
	traverse(r->l, list);
	sortedInsert(list, r);
	traverse(r->r, list);
}

//Ausgeben der Wordclouds
void printList(struct lNode *n, int limit) {
	struct sortNode *sortedList;
	while (n) {
		if (n->count >= 500) { //Nur falls es mehr als 500 Artikel in dem Magazin gibt
			sortedList = NULL;
			traverse(n->tree, &(sortedList));
			printf("############################\nMagazine: %s\nPublished Articles: %d\n############################\n\n", n->mName, n->count);
			itterateList(sortedList, limit);
			printf("\n\n############################\n\n");
			freeList(sortedList);	//Speicherplatz der Sortierlistewieder freigeben
		}
		n = n->next;
	}
}

/************************************************************************
 *
 *	Verarbeitung des Inhalts der XML-Datei
 *
 ************************************************************************/

//Hilfsfuntion zum extrahieren des Magazinnames aus einem String
char * getMagazineName(char *s) {	
	s = strstr(s, "key=\"journals/"); //Gucken ob der article ein Journal ist
	if (s != NULL) s += 14; else return NULL; //Wenn nicht ist er uns egal und es geht weiter

	char *start = s; //Sonst, wir er extrahiert und zurückgegeben
	while (*s != '/') s++;
	*s = '\0';

	return start;
}

//Hilfsfunktion zum extrahieren des Titels zwischen den beiden Tags: <title></title> aus einem String
char * getTitle(char *s) {	
	s = strstr(s, "<title>"); //Gucken ob es ein Title Tag ist
	char *end = strstr(s, "</title>");

	if (!s || !end) return NULL; //Sonst weiter

	s += 7;  //Wenn ja dann, extrahieren und zurückgeben
	char *start = s;
	while (s != end) s++;
	*s = '\0';

	return start;
}

/************************************************************************
 *
 *	Zeilenweises verarbeiten der Daten
 *  (vgl. http://dblps.uni-trier.de/~ley/kp14/lexikon.c)
 *
 ************************************************************************/
void process_file(char *s) {
	int state = 0;
	char c;
	char *start;
	char *tmp;
	char *magazineName;
	char *title;

	while (c = *s) {
		if (c != '\n') { //Newline ist hier das Trennzeichen (statt wie bei word ein Leerzeichen bzw. "nicht Alphabetisches")
			if (!state) {
				start = s;
				state = 1;
			}
		} else {
			state = 0;			
			if (start) {
				*s = '\0';
				if (tmp = strstr(start, "<article")) { //Handelt es sich um einen Article-Tag?
					magazineName = getMagazineName(tmp); //Wenn ja extrahiere den MagazinNamen
				}
				if ((tmp = strstr(start, "<title>")) && magazineName) {  //Handelt es sich um einen Title-Tag und befindet er sich innerhalb eines magazins
					title = getTitle(tmp); //Wenn ja extrahiere Titel
					if (title) insertOrUpdate(magazineName, title); //Füge den Titel zum entsprechenden Magazin in der Magazin-Liste hinzu (falls er existiert)
				}
				*s = c;
			}
			start = NULL;
		}
		s++;
	}
}

int main(int argc, char *argv[]) {
	magazineList = NULL;
	if (argc == 1) {
		printf("Bitte geben Sie den Pfad zu der Datei dblp.xml an!\n");
		exit(EXIT_FAILURE);
	}

	char *f = read_file(argv[1]);

	if (!f) {
		printf("Datei konnte nicht gelesen werden. Korrekter Pfad?\n");
		exit(EXIT_FAILURE);
	}

	process_file(f); //Start
	printList(magazineList, 30); //Ergebnisausgabe

	exit(EXIT_SUCCESS);
}