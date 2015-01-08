//int heap[11] = {0,91,48,1,80,28,22,15,88,59,5};
int heap[10];


int swap(int i,int j){
	int temp = heap[i];
	heap[i] = heap[j];
	heap[j] = temp;
}

int factor(int n)
{
	if (n==0) 
		return 1;
	else
		return n*factor(n-1);
}

void main()
{
	int a;
	read(a);
	a = factor(a);
	write(a);
}
