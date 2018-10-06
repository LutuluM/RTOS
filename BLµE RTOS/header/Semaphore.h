#ifndef SEMAPHORE_H
#define SEMAPHORE_H

typedef signed int Sema;

void CreateSema(Sema *,unsigned int);
void PendSema(Sema *);
unsigned int AcceptSema(Sema *);
void PostSema(Sema *);

#endif
