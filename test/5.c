/*比较全面的测试例子*/
const int m=7,n=85;
int x=7,y=85,z,q,r;
void multiply()
{
	int a,b;
	a=x; b=y; z=0;
	while(b>0)
	{
		if(b>a)
			z=z+a;
		a=a*2;
		b=b/2;
	}
}

void divide()
{
	int w;
	r=x; q=0; w=y;
	while(w<=r)
		w=2*w;
	while(w>y)
	{
		q=2*q; w=w/2;
		if(w<=r)
		{
			r=r-w;
			q=q+1;
		}
	}
}

void gcd()
{
	int f,g;
	f=x; 
	g=y;
	while(f!=g)
	{
		if(f<g) { g=g-f; }
		if(g<f) { f=f-g; }
	}
}

void main()
{
	x=m;
	y=n; 
	multiply();
	x=25; y=3; divide();
	x=34; y=36; gcd();
}
