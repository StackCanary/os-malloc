#include "myalloc.h"
#include <stdio.h>
#include <signal.h>

#include <pthread.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

// A simple test program to show the  use of the library
// By default, myalloc() simply calls malloc() so this will
// work.
// Kasim Terzic, Sep 2017

void *f(void *arg)
{
  
  char* a = myalloc(800000);
  a[0] = 0x41;
  char* b = myalloc(800000);
  b[0] = 0x42;
  char* c = myalloc(80000);
  c[0] = 0x43;
  char* d = myalloc(800000);
  d[0] = 0x41;
  char* e = myalloc(300000);
  e[0] = 0x42;
  char* f = myalloc(100000);
  f[0] = 0x43;
  char* g = myalloc(400000);
  g[0] = 0x41;
  char* h = myalloc(1024);
  h[0] = 0x42;
  char* i = myalloc(16);
  i[0] = 0x43;

  printf("Free a\n");
  myfree(a);
  printf("Free b\n");
  myfree(b);
  printf("Free c\n");
  myfree(c);
  printf("Free d\n");
  myfree(d);
  printf("Free e\n");
  myfree(e);
  printf("Free f\n");
  myfree(f);
  printf("Free g\n");
  myfree(g);
  printf("Free h\n");
  myfree(h);

  return arg;
}

int main()
{
//test();

  pthread_t thread = 0;
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);

  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);

  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);

    pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);

    pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);

    pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);

    pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);


    pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);
  pthread_create(&thread, NULL, f, NULL);




  char* a = myalloc(512);
  a[0] = 0x41;
  char* b = myalloc(1024);
  b[0] = 0x42;
  char* c = myalloc(16);
  c[0] = 0x43;
  char* d = myalloc(512);
  d[0] = 0x41;
  char* e = myalloc(1024);
  e[0] = 0x42;
  char* f = myalloc(16);
  f[0] = 0x43;
  char* g = myalloc(512);
  g[0] = 0x41;
  char* h = myalloc(1024);
  h[0] = 0x42;


  myfree(a);
  myfree(b);
  myfree(c);
  myfree(d);
  myfree(e);
  myfree(f);
  myfree(g);
  myfree(h);


  sleep(5);
}

