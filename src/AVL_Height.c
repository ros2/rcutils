/*
by Felipe Gabriel
this code goes through the tree and prints the height of AVL tree
this code have all the main functions off a AVL tree 
*/

#include <stdio.h>
#include <stdlib.h>


typedef struct node{
	int num;
	struct node* left;
	struct node* right;
}Node;

typedef Node* TreeBin;

TreeBin * newTree(){
	TreeBin* root = (TreeBin*) malloc(sizeof(TreeBin));
	if(root)
	    (*root) = NULL;

	return root;
}

void clearNode(Node* node){
	if(!node)	return;
	clearNode(node->left);
	clearNode(node->right);
	free(node);
	node = NULL;
}

void clearTree(TreeBin* Tree){
	if(!Tree)	return;
	clearNode(*Tree);
	free(Tree);
}

int isEmpty(TreeBin* Tree){
	if(!Tree)	return 1;
	if(!(*Tree))	return 1;
	return 0;
}

int countNode(TreeBin* Tree){
	int tleft, tright;

	if(isEmpty(Tree))	return 0;

	tleft = countNode(&((*Tree)->left));
	tright = countNode(&((*Tree)->right));

	return (tleft+tright+1);
}

int heightTree(TreeBin* Tree){
	int heightLeft, heightRight;

	if(isEmpty(Tree))	return -1;
	if(isEmpty(&((*Tree)->left)) && isEmpty(&((*Tree)->right)))	return 0;

	heightLeft = heightTree(&((*Tree)->left));
	heightRight = heightTree(&((*Tree)->right));

	if(heightLeft > heightRight)	return (heightLeft + 1);

	return (heightRight + 1);
}

//balancing functions
// [-1, 1] balanced tree
// [-2, 2] not balanced tree
int Balanced(TreeBin *Tree){
	return heightTree(&((*Tree)->left)) - heightTree(&((*Tree)->right));
}

void spinRight(TreeBin *Tree){
	Node *nod;
	nod = (*Tree)->left;
	(*Tree)->left = nod->right;
	nod->right = (*Tree);
	(*Tree) = nod;
}

void spinLeft(TreeBin *Tree){
	Node *nod;
	nod = (*Tree)->right;
	(*Tree)->right = nod->left;
	nod->left = (*Tree);
	(*Tree) = nod;
}

void balance(TreeBin *Tree, int n){
	int eb, it;

	if(!(*Tree))    return;

	eb = Balanced(Tree);
	if(eb >= -1 && eb <= 1)	return;

	else{

		if(n > (*Tree)->num){
			it = Balanced(&((*Tree)->right));

			if(eb <= -2){
				if(it == -1 || !it)	spinLeft(Tree);

				else{	
				    spinRight(&((*Tree)->right)); 
				    spinLeft(Tree);
				}
			}else{

				if(it == 1 || !it)	spinRight(Tree);

				else{	
				    spinLeft(&((*Tree)->right)); 
				    spinRight(Tree);

				}
			}
		}else{

			it = Balanced(&((*Tree)->left));

			if(eb <= -2){
				if(it == -1 || !it)	spinLeft(Tree);
				else{	
				    spinRight(&((*Tree)->left)); 
				    spinLeft(Tree);
				}
			}else{
				if(it == 1 || !it)	spinRight(Tree);
				else{	
				    spinLeft(&((*Tree)->left)); 
				    spinRight(Tree);

				}
			}
		}
		return;
	}
}

void insertTree(TreeBin* Tree, int n){
	Node *nod;
	if(!Tree)	return ;
	if(!(*Tree)){
		nod = (Node*)malloc(sizeof(Node));
		nod->num = n;
		nod->left = NULL;
		nod->right = NULL;
		(*Tree) = nod;
		return;
	}
	if(n > (*Tree)->num)	
	    insertTree(&((*Tree)->right), n);
	else	
	    insertTree(&((*Tree)->left), n);

	balance(Tree, n);
}

//Get the higher element of the left sub-tree
void subleftTree(TreeBin *Tree, TreeBin *next){
	Node *nod;
	if(!((*next)->right)){
		(*Tree)->num = (*next)->num;
		nod = (*next);
		(*next) = (*next)->left;
		free(nod);
	}else{
		subleftTree(Tree, &((*next)->right));
	}
}

void removeNode(TreeBin *Tree){
	Node *nod;
	if(!((*Tree)->left)){
		nod = (*Tree);
		(*Tree) = (*Tree)->right;
		free(nod);
	}else{
		if(!((*Tree)->right)){
			nod = (*Tree);
			(*Tree) = (*Tree)->left;
			free(nod);
		}else{
			subleftTree(Tree, &((*Tree)->left));
		}
	}
}

int removeTree(TreeBin* Tree, int n){
	Node *nod;
	if(isEmpty(Tree))	return 0;
	if((*Tree)->num == n){
		removeNode(Tree);
		balance(Tree, n);
		return 1;
	}
	if((*Tree)->num < n)	removeTree(&((*Tree)->right), n);
	else	removeTree(&((*Tree)->left), n);
	balance(Tree, n);
	return 1;
}

int main(){
	int i, n;
	TreeBin* Tree;
	
	Tree = newTree();

	printf("Insert the amount of numbers to be inserted\n");
	scanf("%d", &n);
	printf("What number you want to insert?\n");
	while(n--){
		scanf("%d", &i);
		insertTree(Tree, i);
	}	
	printf("The height is: %d\n", heightTree(Tree));
	
	
	clearTree(Tree);
	return 0;
}
