# include<stdio.h>

void add_one (int *, double *);
void add_one_test(int x,double y);
int main ( void ){
int a=2 ;
double b =3.0;
printf("a=%d, b= %lf",a,b);
add_one (&a ,&b);
// now a=3 and b =4.0
printf("a=%d,b=%lf",a,b);
add_one_test(a,b);
printf("a=%d,b=%lf",a,b);
}
void add_one (int * x, double * y){
*x=*x+1;
*y=*y +1.0;
}
void add_one_test(int x,double y) {
  x = x + 1;
  y = y + 1.0;
}
