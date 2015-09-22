#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAXLENGTH 200 	// max length of a c-string
#define MAXSUBS 20		// max number of type variables

//----------------------------------------------------------------
// Defining the structure of the tree
//----------------------------------------------------------------

typedef struct node Node;

// Acceptable "types" include "Primitive", "List", "Function", "Variable"
struct node {
	char value[MAXLENGTH];
	char type[MAXLENGTH];
	Node* child;
	Node* sibling;
	Node* returnType;
};

// "Constructor" for new nodes on the heap
Node* newNode (char val[], char typ []) {
	Node* sphere = malloc (sizeof(Node));
	strcpy(sphere->value, val);
	strcpy(sphere->type, typ);
	return sphere;
}

// this doesn't work and causes some problems
/*
void deepFree (Node* victim) {
	// check if there are children; if so, recursively call deepFree
	free(victim);
}*/

// Recursively print the tree, in tree form
void printTree(Node* root, int depth) {
	if (depth == 0) printf("Root    : ");
	int i = 0;
	for (; i < depth; i++) printf("\t");
	printf("%s, %s \n", root->value, root->type);
	if (root->child != NULL) {
		printf("First   : ");
		printTree(root->child,depth+1);
	}
	if (root->returnType != NULL) {
		printf("Return  : ");
		printTree(root->returnType,depth+1);
	}
	if (root->sibling != NULL) {
		printf("Sibling : ");
		printTree(root->sibling, depth);
	}
}

// Recursively print the tree, in type form
void printGeneralUnifier(Node* root) {
	if (!strcmp(root->type, "Function")) { 
		printf("(");
		printGeneralUnifier(root->child);
		printf(") -> ");
		printGeneralUnifier(root->returnType);
	} else if (!strcmp(root->type, "List")) {
		printf("[");
		printGeneralUnifier(root->child);
		printf("]");
	} else {
		printf("%s", root->value);
	}
	if (root->sibling != NULL) {
		printf(", ");
		printGeneralUnifier(root->sibling);
	}
}

//----------------------------------------------------------------
// Structure for remembering substitution rules
//----------------------------------------------------------------

typedef struct subnode SubNode;

struct subnode {
	char name[MAXLENGTH];
	Node* sub;
};

// keep track of all substitutions that have occured
static SubNode forwardList[MAXSUBS];

// given the type var name and what it points to, this will add a substitution
void addReference(char s[], Node* replace) {
	int i = 0;
	for (; i < MAXSUBS; i++)
		if (forwardList[i].sub == NULL) {
			forwardList[i].sub = replace;
			strcpy(forwardList[i].name, s);
			return;
		}
	printf("Maxed out substitution table -- increase MAXSUBS constant.\n");
}

// given the type var name, this will return the substitution
Node* findReference(char s[]) {
	int i = 0;
	for (; i < MAXSUBS; i++)
		if (!strcmp(forwardList[i].name, s)) return forwardList[i].sub;
	return NULL;
}

//----------------------------------------------------------------
// Recursive Descent Tokenization Methods
//----------------------------------------------------------------

// A home-brewed solution to removing leading and trailing whitespaces
char* trimWhiteSpaces(char* s) {
	int i = strlen(s);
	for ( ; i > 0; i--) if (s[i-1] != ' ' && s[i-1] != '\t') break;
	s[i] = '\0';
	while (strlen(s) != 0 && (s[0] == ' ' || s[0] == '\t')) s++;
	return s;
}

// Function prototype to allow for "recursive" calling
Node* isType(char s[]);

// PRIMITIVE_TYPE ::= 'int' | 'real' | 'str';
Node* isPrimitiveType (char s[]) {
	if (!strcmp(s,"int") || !strcmp(s, "real") || !strcmp(s, "str"))
		return newNode(s, "Primitive");
	return NULL;
}

// VARNAME ::= [a-zA-z][a-zA-Z0-9]*;
int isVarName (char s[]) {
	if (strlen(s) < 1) return 0;
	if (!isalpha(s[0])) return 0;
	int i = 0;
	for (; i < strlen(s); i++) 
		if (!isalpha(s[i]) && !isdigit(s[i])) return 0;
	return 1;
}

// TYPEVAR ::= '`' VARNAME;
Node* isTypeVar (char s[]) {
	if (strlen(s) < 2) return 0;
	if (s[0] != '`') return 0;
	char ss[MAXLENGTH];
	int i = 1;
	for (; i < strlen(s); i++) { ss[i-1] = s[i]; }
	ss[strlen(s)-1] = 0;
	if (isVarName(ss))
		return newNode(s, "Variable");
	return NULL;
}

// ARGLIST ::= TYPE ',' ARGLIST | TYPE
Node* isArgList(char s[]) {
	// strip leading and trailing whitespace
	char* ss = malloc (MAXLENGTH * sizeof(char));
	strcpy(ss, s);
	strcpy(s, trimWhiteSpaces(ss));

	// Check for the case of TYPE ',' ARGLIST
	int valid = 0;
	int i = 0;
	for (; i < strlen(ss); i++) {
		if (ss[i] == ',') {
			// check TYPE before the ','
			char sss[MAXLENGTH];
			int j = 0;
			for (; j < i; j++) { sss[j] = ss[j]; }
			sss[i] = 0;
			char* ssbefore = malloc (MAXLENGTH * sizeof(char));
			strcpy(ssbefore, sss);
			strcpy(sss, trimWhiteSpaces(ssbefore));
			// check ARGLIST after the ','
			char ssss[MAXLENGTH];
			j = i+1;
			for (; j < strlen(ss); j++) { ssss[j-i-1] = ss[j]; }
			ssss[strlen(ss)-i-1] = 0;
			char* ssafter = malloc (MAXLENGTH * sizeof(char));
			strcpy(ssafter, ssss);
			strcpy(ssss, trimWhiteSpaces(ssafter));
			free(ssbefore);
			free(ssafter);

			Node* holder;
			// if the first argument works, remember that it worked for this round
			if ((holder=isType(sss))!=NULL) { valid = 1; }

			Node* holderSib;
			// if the remainder of the arglist fails, fail the valid check
			// we don't want to return here, because the for loop may still check other possibilities
			if ((holderSib=isArgList(ssss))==NULL) { valid = 0; }
			// otherwise, append to holder's siblings
			else { 
				if (valid) {
					holder->sibling = holderSib;
					return holder;
				}
			}
		}
	}

	// Check for the case of TYPE
	Node* holder;
	if ((holder=isType(ss))!=NULL) { return holder; }
	return NULL;
}

// FUNCTYPE ::= '(' ARGLIST ')' '->' TYPE | '(' ')' '->' TYPE;
Node* isFuncType(char s[]) {
	if (strlen(s) < 6) return NULL;
	if (s[0] != '(') return NULL;
	Node* functionNode = newNode("()", "Function");

	// Check for '('
	char ss[MAXLENGTH];
	int i = 1;
	for (; i < strlen(s); i++) { ss[i-1] = s[i]; }
	ss[strlen(s)-1] = 0;
	char* sss = ss;
	// Trim out leading whitespaces
	while (strlen(sss) != 0 && (sss[0] == ' ' || sss[0] == '\t')) sss++;
	// Test for arglist based on existence of ')'
	int endpos = 0;
	for (; endpos < strlen(sss); endpos++) {
		if (sss[endpos] == ')') {
			char ssss[MAXLENGTH];
			i = 0;
			for (; i < endpos; i++) { ssss[i] = sss[i]; }
			ssss[endpos] = 0;
			Node* childHolder;
			if ((childHolder=isArgList(ssss)) || !strcmp(ssss,"")) {
				// we should only be appending to functionNode here, not within isArgList
				functionNode->child = childHolder;
				break; // this means that we have found a valid arglist
			}
		}
	}

	if (endpos == strlen(sss)) return NULL;
	if (sss[endpos] != ')') return NULL;

	// get rid of the ) character
	char remain[MAXLENGTH];
	i = endpos+1; 
	for (; i < strlen(sss); i++) { remain[i-endpos-1] = sss[i]; }
	remain[strlen(sss)-endpos-1] = 0;
	char* remain2 = remain;
	while (strlen(remain2) != 0 && (remain2[0] == ' ' || remain2[0] == '\t')) remain2++;

	// get rid of the -> characters
	char remain3[MAXLENGTH];
	strcpy(remain3, remain2);
	if (strlen(remain3) < 4 || (remain3[0] != '-' && remain3[1] != '>')) return NULL;
	char returnWithSpaces[MAXLENGTH];
	i = 2;
	for (; i < strlen(remain3); i++) { returnWithSpaces[i-2] = remain3[i]; }
	returnWithSpaces[strlen(remain3)-2] = 0;

	// will just have the return arguments here now
	char* returnVals = returnWithSpaces;
	while (strlen(returnVals) != 0 && (returnVals[0] == ' ' || returnVals[0] == '\t')) returnVals++;

	if ( (functionNode->returnType = isType(returnVals)) != NULL)  return functionNode;

	return NULL;
}

// LISTTYPE ::= '[' TYPE ']'
Node* isListType(char s[]) {
	if (strlen(s) < 4) return NULL;
	if (s[0] != '[') return NULL;
	if (s[strlen(s)-1] != ']') return NULL;
	char ss[MAXLENGTH];
	int i = 1;
	for (; i < strlen(s)-1; i++) { ss[i-1] = s[i]; }
	ss[strlen(s)-2] = 0;
	char* sss = malloc (MAXLENGTH * sizeof(char));
	strcpy(sss, ss);
	strcpy(ss, trimWhiteSpaces(sss));
	free(sss);
	Node* listNode = newNode("[]","List");
	if ((listNode->child = isType(ss)) != NULL) return listNode;
	return NULL;
}

// TYPE ::= TYPEVAR | PRIMITIVE_TYPE | FUNCTYPE | LISTTYPE
Node* isType (char s[]) {
	Node* root;
	if ((root = isPrimitiveType(s)) != NULL) return root;
	if ((root = isTypeVar(s)) != NULL) return root;
	if ((root = isFuncType(s)) != NULL) return root;
	if ((root = isListType(s)) != NULL) return root;
	return NULL;
}

// UNIFICATION_QUERY ::= TYPE ‘^’ TYPE;
int isUnificationQuery(char s[], Node** lhsNode, Node** rhsNode) {
	if (strlen(s) == 0) return 0;
	char* testcaret = strchr(s, '^');
	if (testcaret == NULL) return 0;

	// find where the caret occured
	int splitindex = 0;
	for (; splitindex < strlen(s); splitindex++)
		if (s[splitindex] == '^') break;

	// Build the left tree
	char lhs[MAXLENGTH];
	int i = 0;
	for (; i < splitindex; i++) lhs[i] = s[i];
	lhs[splitindex] = 0;
	char* lhsremove = malloc (MAXLENGTH * sizeof(char));
	strcpy(lhsremove, lhs);
	strcpy(lhs, trimWhiteSpaces(lhsremove));
	free(lhsremove);
	*lhsNode = isType(lhs);
	if (*lhsNode == NULL) return 0;

	// Build the right tree
	char rhs[MAXLENGTH];
	i = splitindex+1;
	for (; i < strlen(s); i++) rhs[i-splitindex-1] = s[i];
	rhs[strlen(s)-splitindex-1] = 0;
	char* rhsremove = malloc (MAXLENGTH * sizeof(char));
	strcpy(rhsremove, rhs);
	strcpy(rhs, trimWhiteSpaces(rhsremove));
	free(rhsremove);
	*rhsNode = isType(rhs);
	if (*rhsNode == NULL) return 0;

	return 1;
}

//----------------------------------------------------------------
// Unification Methods
//----------------------------------------------------------------

// go through the tree: if 'var' is found, then a cycle exists
int cycleExist (Node* tree, Node* var) {
	if (!strcmp(tree->type, "Function")) { 
		if (cycleExist(tree->child, var)) return 1;
		if (cycleExist(tree->returnType, var)) return 1;
	} else if (!strcmp(tree->type, "List")) {
		if (cycleExist(tree->child, var)) return 1;
	} else {
		if (!strcmp(tree->value,var->value)) return 1;
	}
	if (tree->sibling != NULL)
		if (cycleExist(tree->sibling, var)) return 1;
	return 0;
}

int unification (Node* lhs, Node* rhs) {

	// if neither node is a type variable
	if (strcmp(lhs->type,"Variable") && strcmp(rhs->type,"Variable")) {
		//printf("Comparing %s and %s\n", lhs->value, rhs->value);
		// ask if the nodes are the same
		if (strcmp(lhs->type,rhs->type)) return 0;
		//printf("%s and %s are the same type\n", lhs->type, rhs->type);
		if (strcmp(lhs->value,rhs->value)) return 0;
		//printf("%s and %s are the same value\n", lhs->value, rhs->value);
		// walk the tree at this point
		// check the children (go into depth)
		if (lhs->child == NULL && rhs->child != NULL) return 0;
		if (lhs->child != NULL && rhs->child == NULL) return 0;
		if (lhs->child != NULL && rhs->child != NULL)
			if (!unification(lhs->child, rhs->child)) return 0;
		// check the siblings (go into breadth)
		if (lhs->sibling == NULL && rhs->sibling != NULL) return 0;
		if (lhs->sibling != NULL && rhs->sibling == NULL) return 0;
		if (lhs->sibling != NULL && rhs->sibling != NULL)
			if (!unification(lhs->sibling, rhs->sibling)) return 0;
		// check the return type (for functions)
		if (lhs->returnType != NULL && rhs->returnType != NULL)
			if (!unification(lhs->returnType, rhs->returnType)) return 0;
	}

	// if either is a type variable, make substitution
	if (!strcmp(lhs->type, "Variable")) {
		Node* forward;
		while ((forward=findReference(lhs->value))!=NULL) {
			//printf("%s is replaced by %s\n", lhs->value, forward->value);
			strcpy(lhs->value, forward->value);
			strcpy(lhs->type,forward->type);
			lhs->child = forward->child;
			lhs->returnType = forward->returnType;
		}
	}
	if (!strcmp(rhs->type, "Variable")) {
		Node* forward;
		while ((forward=findReference(rhs->value))!=NULL) {
			//printf("%s is replaced by %s\n", rhs->value, forward->value);
			strcpy(rhs->value,forward->value);
			strcpy(rhs->type,forward->type);
			rhs->child = forward->child;
			rhs->returnType = forward->returnType;
		}
	}

	// if both are type variables, pick one to dominate
	// if they are the same, ignore
	if (!strcmp(lhs->type, rhs->type) && !strcmp(lhs->type,"Variable")) {
		if (strcmp(lhs->value, rhs->value)) {
			//printf("%s and %s will be unified.\n", lhs->value, rhs->value);
			// remember the two values
			addReference(rhs->value, lhs);
			//printf("%s is the winner\n", findReference(rhs->value)->value);
			strcpy(rhs->value,lhs->value);
		} else {
			//printf("Same type variable.\n");
		}
	}

	// if only one node is a type variable, follow forwarding links to change type
	// be sure to adopt children BUT NOT SIBLINGS
	if (strcmp(lhs->type, rhs->type) && (!strcmp(lhs->type, "Variable") || !strcmp(rhs->type, "Variable"))) {
		//printf("%s and %s will be unified.\n", lhs->value, rhs->value);
		// if the type variable is on the left, unify with the right
		if (!strcmp(lhs->type,"Variable")) {
			// the left node is a variable: check right to see if recursive
			if (cycleExist(rhs, lhs)) return 0;
			// remember the lhs value
			addReference(lhs->value, rhs);
			//printf("%s is the winner\n", findReference(lhs->value)->value);
			strcpy(lhs->type,rhs->type);
			strcpy(lhs->value,rhs->value);
			lhs->child = rhs->child;
			lhs->returnType = rhs->returnType;
		} else {
			// the right node is a variable: check left to see if recursive
			if (cycleExist(lhs, rhs)) return 0;
			// remember the rhs value
			addReference(rhs->value, lhs);
			//printf("%s is the winner\n", findReference(rhs->value)->value);
			strcpy(rhs->type,lhs->type);
			strcpy(rhs->value,lhs->value);
			rhs->child = lhs->child;
			rhs->returnType = lhs->returnType;
		}
	}

	// if both are now non-type variables, do the recursive comparison
	if (strcmp(lhs->type,"Variable") && strcmp(rhs->type,"Variable")) {
		//printf("2. Comparing %s and %s\n", lhs->value, rhs->value);
		// ask if the nodes are the same
		if (strcmp(lhs->type,rhs->type)) return 0;
		//printf("%s and %s are the same type\n", lhs->type, rhs->type);
		if (strcmp(lhs->value,rhs->value)) return 0;
		//printf("%s and %s are the same value\n", lhs->value, rhs->value);
		// walk the tree at this point
		// check the children (go into depth)
		if (lhs->child == NULL && rhs->child != NULL) return 0;
		if (lhs->child != NULL && rhs->child == NULL) return 0;
		if (lhs->child != NULL && rhs->child != NULL)
			if (!unification(lhs->child, rhs->child)) return 0;
		// check the siblings (go into breadth)
		/*
		if (lhs->sibling == NULL && rhs->sibling != NULL) return 0;
		if (lhs->sibling != NULL && rhs->sibling == NULL) return 0;
		if (lhs->sibling != NULL && rhs->sibling != NULL)
			if (!unification(lhs->sibling, rhs->sibling)) return 0;
		*/
		// check the return type (for functions)
		if (lhs->returnType != NULL && rhs->returnType != NULL)
			if (!unification(lhs->returnType, rhs->returnType)) return 0;
		// return 1;
	}

	// if either node is still a type variable, add a forwarding link
	if (!strcmp(lhs->type, "Variable")) {
		Node* forward;
		while ((forward=findReference(lhs->value))!=NULL) {
			//printf("%s is replaced by %s\n", lhs->value, forward->value);
			strcpy(lhs->value, forward->value);
			strcpy(lhs->type,forward->type);
			lhs->child = forward->child;
			lhs->returnType = rhs->returnType;
		}
	}
	if (!strcmp(rhs->type, "Variable")) {
		Node* forward;
		while ((forward=findReference(rhs->value))!=NULL) {
			//printf("%s is replaced by %s\n", rhs->value, forward->value);
			strcpy(rhs->value,forward->value);
			strcpy(rhs->type,forward->type);
			rhs->child = forward->child;
			rhs->returnType = lhs->returnType;
		}
	}

	// even if both are type variables, we still have siblings to check through
	if (lhs->sibling == NULL && rhs->sibling != NULL) return 0;
	if (lhs->sibling != NULL && rhs->sibling == NULL) return 0;
		if (lhs->sibling != NULL && rhs->sibling != NULL)
			if (!unification(lhs->sibling, rhs->sibling)) return 0;

	return 1;
}

//----------------------------------------------------------------
// Main
//----------------------------------------------------------------

int main() {
	char s [MAXLENGTH];
	int flag = 0;
	while (fgets(s, sizeof(s), stdin)) {
		// Strip the newline that we receive
		if (strlen(s) > 1 && s[strlen(s)-1] == '\n') s[strlen(s)-1] = 0;
		// Strip leading and trailing whitespaces
		char* ss = malloc (MAXLENGTH * sizeof(char));
		strcpy(ss, s);
		strcpy( s, trimWhiteSpaces(ss) );
		free(ss);
		// Check if this line gave us QUIT
		if (!strcmp(s,"QUIT"))
			break;
		Node* lhs;
		Node* rhs;
		// Otherwise, we will look through this string
		if (isUnificationQuery(s, &lhs, &rhs)) {
			//printTree(lhs,0);
			//printTree(rhs,0);
			if (!unification(lhs,rhs)) { flag = 1; break; }
			printGeneralUnifier(lhs);
			printf("\n");
		} else
			break;
		// deepFree(lhs);
	}
	// If an error was detected, print ERR
	if (flag == 1) printf("BOTTOM\n");
	else if (strcmp(s,"QUIT")) printf("ERR\n");
	return 0;
}