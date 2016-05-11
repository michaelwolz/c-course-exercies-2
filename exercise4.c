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

struct node {
	char		*w;
	int	 	count;
	struct node	*l, *r;
} *tree;

struct lNode {
	int 	count;
	char 	*mName;
	struct lNode	*next;
	struct node 	*tree;
} *magazineList;

void sortTree(struct node **root) {
	
}

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

void words(struct node **root, char *s)
{
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

void update(struct lNode *n, char *title) {
	words(&(n->tree), title);
	n->count++;
}

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

void traverse(struct node *r) {
	if (r == NULL) 
		return;
	
	traverse(r->l);
	if (r->count > 15) 
		printf("%d %s\n", r->count, r->w);
	traverse(r->r);
}

void out_list(struct lNode *n) {
	while (n) {
		if (n->count >= 500) {
			printf("############################\nMagazine: %s\nPublished Articles: %d\n############################\n\n", n->mName, n->count);
			traverse(n->tree);
			printf("\n############################\n\n");
		}
		n = n->next;
	}
}

char * getMagazineName(char *s) {	
	s = strstr(s, "key=\"journals/");
	if (s != NULL) s += 14; else return NULL;

	char *start = s;
	while (*s != '/') s++;
	*s = '\0';

	return start;
}

char * getTitle(char *s) {	
	s = strstr(s, "<title>");
	char *end = strstr(s, "</title>");

	if (!s || !end) return NULL;

	s += 7;
	char *start = s;
	while (s != end) s++;
	*s = '\0';

	return start;
}

void process_file(char *s)
{
	int state = 0;
	char c;
	char *start;
	char *tmp;
	char *magazineName;
	char *title;

	while (c = *s) {
		if (c != '\n') {
			if (!state) {
				start = s;
				state = 1;
			}
		} else {
			state = 0;			
			if (start) {
				*s = '\0';
				if (tmp = strstr(start, "<article")) {
					magazineName = getMagazineName(tmp);
					if (!magazineName) continue;
				} else if ((tmp = strstr(start, "<title>"))) {
					title = getTitle(tmp);
					if (!title) continue;
					insertOrUpdate(magazineName, title);
				}
				*s = c;
			}
			start = NULL;
		}
		s++;
	}
}

/************************************************************************
 *
 * 	Loads a record completely into the memory.
 *
 ************************************************************************/

char *read_file(char *file_name)
{
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

int main(int argc, char *argv[]) {
	tree = NULL;
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

	process_file(f);
	out_list(magazineList);
	//traverse(tree);

	exit(EXIT_SUCCESS);
}