void fchar(char c);
void fint(int i);
void flong(long i);
void ffloat(float f);
void fdouble(double f);

void fall(char a, int i, long t, long long b, float f, double d, void *p);

argh()
{
  fchar('a');
  fint(3);
  flong(3l);
  ffloat(3.14f);
  fdouble(3.14);    

  fall('a', 3, 3l, 4LL, 3.14f, 3.14, &argh);
}
