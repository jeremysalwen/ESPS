/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)methods.h	1.2 9/26/95 ATT/ERL/ESI */
/* methods.h */

struct methods {
  char *name;
  char *(*meth)();
  struct methods *next;
};
#define Methods struct methods


struct selector {
  char *name;
  char *format;
  char *dest;
  struct selector *next;
};
#define Selector struct selector
