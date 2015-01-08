/*
	notice the local varible,  and the decl stmt could be write in 'anywhere';
*/
//int *a,b,c,(*d)[10],g[10];
//int a=2,t;
int d[10];
typedef int * intp;
int a,b;
int *p;
float f;
//float f,b[10];
//char c;
char *s1,*s2,*s3;
intp maxp(int*a,int*b)
{
	if (*a>=*b) return a;
	else return b;
}
/*int max(int a,int b)
{
	if (a>=b) return a;
	else return b;
}*/
//int b,a[10][5];
//float *c;
int main()
{
//	read(&b[8]);
//	write(b[8]);
//	write('\n');
//	p=&a;
//	*p=3;
int i=1;
	{
	int i=0;
	while (i<10) {
		write('\n');
		s1="Please Input A = ";  //This can be dump but not run.>_<
		s2="Please Input B = ";
		write(s1);
		read(&a);
		write('\n');
		write(s2);
		read(&b);
		write('\n');
		if ((a==0)&&(b==0)) break;
	//	s3=;
		write("Maxp(A,B) = ");
		if (maxp(&a,&b)==&a)
			write('A');
		else	write('B');
		d[i]=*maxp(&a,&b);
		f = f + d[i]/2.0;
		write('(');
		write(d[i]);
		write(')');
		write('\n');
		
		write("Half Sum : ");
		write(f);
		write('\n');
		i=i+1;
	}
	write("C1 Test Finish!");
	write('\n');
	}
//	b;
//	a=2.5;
//	b[7]=10;
//	f=skip(2,b[7]*3.0);
//	f=skip(10,5.0);
//	c='a';
//	p=&a;
//	*p=3;
//	read &a;
//	"asdas"
//	f=0.5;
/*	int i=0;
	while (f<11)
	{
		b[i]=f;
		i=i+1;
		if (i>=10) break;
		f=f+1;
	}*/
//	skip(&a,t);
//	t=a;
//	read &b[8];
//	b[8];
//	write f;
//	write '\n';
//	a=2.0;
//	f=3;
//	f=3.5*a;
/*	b=7;
	a=&b;
	c=*a;
	*a=128;
	d=g;
//	b[7][2]=4;
//	c=d[3];
	(*d)[1]=3;
//	*d[*d[0][0]][*&b]=a;
//	f=3;
//	d=5;
	if (a<b)
		while (a<c)
			skip(3);
	else a=skip(c);*/
}
