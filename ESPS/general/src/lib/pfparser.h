/* Volker Strom CSTR 2007-09-11 */

#ifdef SUN386i
static struct fnode * fcons(float  v, struct fnode *list);
#else
static struct fnode * fcons(double v, struct fnode *list);
#endif
static struct inode * icons(int v, struct inode *list);
static struct cnode * ccons(char *v, struct cnode *list);
static bad_decl(char *name);
static set_choice(struct cnode *v);

