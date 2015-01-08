typedef struct node *Node;
struct node
{
	int data;
	Node pNext;
} root,n1;

Node LinkNode(Node p,Node p2)
{
	p->pNext = p2;
	return p;
}

void main()
{
	int D[5] = {5,3,4,1,2};
	struct node n2 = {18,7}; 
	Node (*pFunc)(Node,Node) = & LinkNode;
	write("please input the root.data : ");
	read(root.data);
	(*pFunc)(&root,&n1);
	root.pNext->data = root.data;
	write("\nn1.data = ");
	write(n1.data);
	write("\nn2.data = ");
	write(n2.data);
	write("\n");
}