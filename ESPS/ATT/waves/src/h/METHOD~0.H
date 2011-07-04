/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* Methods.h */
/* @(#)Methods.h	1.2 9/26/95 ATT/ERL/ESI */


typedef struct methods {
  char *name;
  char *(*meth)();
  struct methods *next;
} Methods;

typedef struct selector {
  char *name;
  char *format;
  char *dest;
  struct selector *next;
} Selector;

