/*函数调用*/

int x,y,z;
void func2()
{
 z=x+y;
}
void func1()
{
 func2(); 
}

void main()
{
  x = 6;
  y = 7;
  func1();
}


