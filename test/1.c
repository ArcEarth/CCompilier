/*Test file for Principle of Compilier*/

// variable initialization
static float k = 4.5 * 6;

// Initializer-List & Constant folding
int Data[] = { 1 , 2, 4, 5, -7, 8+3};

// String Literal & mul-demension array initialize
char Names[][10] = { "bill" , "steven" , "something"}; 

//Forward declaration of struct type
struct foo;

//Typedef and reference incomplete type
typedef struct foo *foo_ptr;

struct foo{
	// Nested Record Type 
	struct bar
	{
		int ia,ib,ic;
	} ba;

	// offset indicator
	int i : 2;
	float f : 1;

	// Reference self-type in pointer form
	foo_ptr next;
} *a, *b[5];

int n,f;

// Function Declaraton & Parameters
int factorial(int n);

int factorial(int n){
	if(n==1)	return 1;
	if(n>1){
		return n*factorial(n-1);
	}
}

void main(){
	// An err-refernece
	pFoo = a;

	const int ci = 5;

	ci = 6;

	a = *ci;

	a = a->next;
	//write(x);
	a->ba.ia = n;
	b[3] = a;
	f=1;
	//factorial(5);
}


