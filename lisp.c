/******************************************************************************/
/* MakerLisp                                                                  */
/*                                                                            */
/* Source preview                                                             */
/*                                                                            */
/* Compile on Linux:                                                          */
/* cc -olisp -O3 -Wall -I. lisp.c platform.c -lm                              */
/*                                                                            */
/******************************************************************************/

#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <platform.h>

/******************************************************************************/
/* Data management                                                            */
/******************************************************************************/

/* Virtual machine instruction routines */
static void c_apply();
static void c_argc();
static void c_continue();
static void c_define();
static void c_end();
static void c_evalc();
static void c_get00();
static void c_get10();
static void c_get20();
static void c_getb();
static void c_getl();
static void c_jumpc();
static void c_jumpm();
static void c_lambda();
static void c_load();
static void c_loadc();
static void c_macro();
static void c_makecc();
static void c_quote();
static void c_select();
static void c_setb();
static void c_setl();

/* Atomic data cell types */
#define NILTYPE (DCELL *)c_end
#define INTNTYP (DCELL *)-1
#define NUMBTYP (DCELL *)-2
#define SYMBTYP (DCELL *)-3
#define ADDRTYP (DCELL *)-4
#define CONTYPE (DCELL *)-5
#define VECTYPE (DCELL *)-6
#define LASTYPE VECTYPE

/* Data cell header */
#define FORWARD(x) (x)->header.forward
#define TYPE(x) (x)->header.type

/* Predicates */
#define NILP(x) (TYPE(x) == NILTYPE)
#define INTNP(x) (TYPE(x) == INTNTYP)
#define NUMBP(x) (TYPE(x) == NUMBTYP)
#define SYMBP(x) (TYPE(x) == SYMBTYP)
#define ADDRP(x) (TYPE(x) == ADDRTYP)
#define CONTP(x) (TYPE(x) == CONTYPE)
#define VECTP(x) (TYPE(x) == VECTYPE)
#define ATOMD(x) (!(TYPE(x) < LASTYPE))
#define ATOMP(x) (NILP(x) || ATOMD(x))
#define PAIRP(x) (!ATOMP(x))
#define CMNDV(x) ((char *)x < BSSDATA)
#define CMNDP(x) CMNDV(CAR(x))
#define PAIRD(x) (!((ptrdiff_t)TYPE(x) < (ptrdiff_t)BSSDATA))

/* Atomic data access */
#define INTNF(x) (x)->data.i
#define NUMBF(x) (x)->data.f
#define SYMBL(x) (x)->data.l
#define SYMBN(x) ((char *)&(x)->data.l + sizeof((x)->data.l))
#define ADDRF(x) (x)->data.a
#define CONTF(x) (x)->data.k
#define VECTL(x) (x)->data.vec.l
#define VECTF(x) (x)->data.vec.v
#define VREFF(x, i) \
    CAR((DCELL *)((char *)VECTF(x) + \
                  i*(sizeof((x)->header) + sizeof((x)->data.cdr))))

/* List operations */
#define CAR(x) TYPE(x)
#define CDR(x) (x)->data.cdr
#define CONS(x, y) mkpair(x, y)

/* Stack operations */
#define TOP(s) CAR(s)
#define PUSH(s, v) s = CONS(v, s)
#define DROP(s) s = CDR(s)
#define POP(s, v) v = CAR(s); DROP(s)
#define EMPTY(s) s = nil

/* Symbol operations */
#define EQNAM(x, n) (!strcmp(SYMBN(x), n))
#define MKSYMB(s) mksymb(s, strlen(s))

/* Mark top environment change */
#define TOPCHG(a) topchg = intop(a) ? 1 : topchg

/* Data cell definition */
typedef struct dcell {
    struct {
        struct dcell *forward;
        struct dcell *type;
    } header;
    union {
        ptrdiff_t i;
        double f;
        size_t l;
        struct dcell *a;
        struct dcell *cdr;
        struct {
            size_t l;
            struct dcell *v;
        } vec;
        struct {
            struct dcell *s;
            struct dcell *e;
            struct dcell *c;
        } k;
    } data;
} DCELL;

/* Heap */
#define GUARDPAGE 1280*(3*sizeof(DCELL *))
#define HALLOCSZ (HEAPHALF*2 + GUARDPAGE)
static char heap[HALLOCSZ] ALIGNED(HEAPBMOD);
static volatile char *hp;

/* Heap management state */
static int topchg;
static unsigned long gccnt, ccells, maxbytes, maxcells, topbytes, topcells;
static ptrdiff_t heapsize;
static char *heapbase, *heapend, *heaplim, *heapstart, *other;
static char *topbase, *topend;

/* Root virtual machine state */
static DCELL *S, *E, *C, *D;

/* Other necessarily persistent values */
static DCELL *topenv, *nil, *tval, *unval;

/* Symbol table */
static DCELL *T;

/* Break (^C) handling */
static int dbrk;
static void deferbrk()
{
    dbrk = 1;
}

/* Symbol string sizes put symbols on good boundaries */
static size_t sstrsz(size_t n)
{
    int i;

    i = n + 1;
    if (i % SSTRINC) {
        i += SSTRINC - (i % SSTRINC);
    }

    return i;
}

/* Make data of all types */
static DCELL *mknil()
{
    DCELL *v;

    v = (DCELL *)hp;
    FORWARD(v) = v;
    TYPE(v) = NILTYPE;
    hp = (char *)v + sizeof(v->header);

    return v;
}
static DCELL *mkpair(DCELL *a, DCELL *d)
{
    DCELL *v;

    v = (DCELL *)hp;
    FORWARD(v) = v;
    CAR(v) = a;
    CDR(v) = d;
    hp = (char *)v + sizeof(v->header) + sizeof(v->data.cdr);

    return v;
}
static DCELL *mkintn(ptrdiff_t i)
{
    DCELL *v;

    v = (DCELL *)hp;
    FORWARD(v) = v;
    TYPE(v) = INTNTYP;
    INTNF(v) = i;
    hp = (char *)v + sizeof(v->header) + sizeof(v->data.i);

    return v;
}
static DCELL *mknumb(double f)
{
    DCELL *v;

    v = (DCELL *)hp;
    FORWARD(v) = v;
    TYPE(v) = NUMBTYP;
    NUMBF(v) = f;
    hp = (char *)v + sizeof(v->header) + sizeof(v->data.f);

    return v;
}
static DCELL *mksymb(char *s, size_t n)
{
    size_t m;
    DCELL *t, *v;

    /* Symbol already there ? */
    t = T;
    while (!NILP(t)) {
        v = CAR(t);
        m = SYMBL(v);
        if ((m == n) && !memcmp(SYMBN(v), s, m))  {
            return v;
        }
        t = CDR(t);
    }

    /* Make symbol */
    v = (DCELL *)hp;
    FORWARD(v) = v;
    TYPE(v) = SYMBTYP;
    memcpy(SYMBN(v), s, n);
    *(SYMBN(v) + n) = '\0';
    SYMBL(v) = n;
    hp = (char *)v + sizeof(v->header) + sizeof(v->data.l) + sstrsz(n);

    /* Put in symbol table */
    PUSH(T, v);
 
    return v;
}
static DCELL *mkaddr(DCELL *addr)
{
    DCELL *v;

    v = (DCELL *)hp;
    FORWARD(v) = v;
    TYPE(v) = ADDRTYP;
    ADDRF(v) = addr;
    hp = (char *)v + sizeof(v->header) + sizeof(v->data.a);

    return v;
}
static DCELL *mkcont(DCELL *s, DCELL *e, DCELL *c)
{
    DCELL *v;

    v = (DCELL *)hp;
    FORWARD(v) = v;
    TYPE(v) = CONTYPE;
    CONTF(v).s = s;
    CONTF(v).e = e;
    CONTF(v).c = c;
    hp = (char *)v + sizeof(v->header) + sizeof(v->data.k);

    return v;
}
static DCELL *mkvect(DCELL *list)
{
    char *h;
    size_t n;
    DCELL *l, *r, *v;

    v = (DCELL *)hp;
    FORWARD(v) = v;
    TYPE(v) = VECTYPE;
    h = (char *)v + sizeof(v->header) + sizeof(v->data.vec);

    /* Copy list to make vector */
    l = list;
    n = 0;
    VECTF(v) = (DCELL *)h;
    while (!NILP(l)) {
        r = (DCELL *)h;
        h += sizeof(v->header) + sizeof(v->data.cdr);
        FORWARD(r) = r;
        CAR(r) = CAR(l);
        CDR(r) = NILP(CDR(l)) ? nil : (DCELL *)h;
        ++n;
        DROP(l);
    }
    VECTL(v) = n;
    if (!n) {
        VECTF(v) = nil;
    }
    hp = h;

    return v;
}

/* Is the item in the (corraled) top environment ? */
static int intop(DCELL *d)
{
    if (!((char *)d < topbase) && ((char *)d < topend)) {
        return 1;
    }

    return 0;
}

/* Is the item in the current heap ? */
static int inheap(DCELL *d)
{
    if (!((char *)d < heapbase) && ((char *)d < heapend)) {
        return 1;
    }

    return 0;
}

/* Copy data from old heap to new heap */
static void init_data();
static void init_eval();
static void repljmp();
static DCELL *copycell(DCELL *d)
{
    /* Commands are not values on the heap */
    if (CMNDV(d)) {
        return d;
    }

    /* In corraled top environment ? */
    if (intop(FORWARD(d))) {
        return FORWARD(d);
    }

    /* Or already on the new heap ? */
    if (inheap(FORWARD(d))) {
        return FORWARD(d);
    }

    /* Past the end, into the margin ? */
    if (!(hp < &heap[HALLOCSZ - GUARDPAGE])) {
        init_data();
        init_eval();
        fputs("? Out of memory during GC\n", stdout);
        repljmp();
    }

    /* Move, leave forwarding address on front door */
    ++ccells;

    /* NIL */
    if (NILP(d)) {
        FORWARD(d) = mknil();
        return FORWARD(d);
    }

    /* Integer */
    if (INTNP(d)) {
        FORWARD(d) = mkintn(INTNF(d));
        return FORWARD(d);
    }

    /* Real number */
    if (NUMBP(d)) {
        FORWARD(d) = mknumb(NUMBF(d));
        return FORWARD(d);
    }

    /* Symbol */
    if (SYMBP(d)) {
        FORWARD(d) = mksymb(SYMBN(d), SYMBL(d));
        return FORWARD(d);
    }

    /* Address */
    if (ADDRP(d)) {
        FORWARD(d) = mkaddr(ADDRF(d));
        return FORWARD(d);
    }

    /* Continuation */
    if (CONTP(d)) {
        FORWARD(d) = mkcont(CONTF(d).s, CONTF(d).e, CONTF(d).c);
        return FORWARD(d);
    }

    /* Vector */
    if (VECTP(d)) {
        FORWARD(d) = mkvect(VECTF(d));
        return FORWARD(d);
    }

    /* Pair */
    FORWARD(d) = mkpair(CAR(d), CDR(d));
    return FORWARD(d);
}

/* Expand contents of new heap until it's all there */
static void expheap(char *start)
{
    char *sp;
    DCELL *dp, *r;

    sp = start;
    while (sp < hp) {
        dp = (DCELL *)sp;
        if (NILP(dp)) {
            sp += sizeof(dp->header);
            continue;
        }
        if (INTNP(dp)) {
            sp += sizeof(dp->header) + sizeof(dp->data.i);
            continue;
        }
        if (NUMBP(dp)) {
            sp += sizeof(dp->header) + sizeof(dp->data.f);
            continue;
        }
        if (SYMBP(dp)) {
            sp += sizeof(dp->header) + sizeof(dp->data.l) + sstrsz(dp->data.l);
            continue;
        }
        if (ADDRP(dp)) {
            ADDRF(dp) = copycell(ADDRF(dp));
            sp += sizeof(dp->header) + sizeof(dp->data.a);
            continue;
        }
        if (CONTP(dp)) {
            CONTF(dp).s = copycell(CONTF(dp).s);
            CONTF(dp).e = copycell(CONTF(dp).e);
            CONTF(dp).c = copycell(CONTF(dp).c);
            sp += sizeof(dp->header) + sizeof(dp->data.k);
            continue;
        }
        if (VECTP(dp)) {
            r = VECTF(dp);
            sp += sizeof(dp->header) + sizeof(dp->data.vec);
            while (!NILP(r)) {
                CAR(r) = copycell(CAR(r));
                sp += sizeof(dp->header) + sizeof(dp->data.cdr);
                DROP(r);
            }
            continue;
        }
        CAR(dp) = copycell(CAR(dp));
        CDR(dp) = copycell(CDR(dp));
        sp += sizeof(dp->header) + sizeof(dp->data.cdr);
    }
}

/* Copy a root, then expand it */
static DCELL *copyroot(DCELL *r)
{
    char *sp;
    DCELL *n;

    sp = (char *)hp;
    n = copycell(r);
    expheap(sp);

    return n;
}

/* Make a new heap */
static void newheap()
{
    /* Statistics */
    ++gccnt;

    /* Are we in top half ? */
    if (!(heapbase == heapstart)) {

        /* New heap is in bottom half */
        heapbase = heapstart;
        heapend = heapbase + heapsize;
        heaplim = heapend - GUARDPAGE;
        other = heapstart + heapsize;
        hp = heapbase;
    } else {

        /* New heap is in top half */
        heapbase = heapstart + heapsize;
        heapend = heapbase + heapsize;
        heaplim = heapend - GUARDPAGE;
        other = heapstart;
        if (hp < heapbase) {
            hp = heapbase;
        } else {
            heapbase = (char *)hp;
        }
    }
}

/* Copy and corral top environment */
static void copytop()
{
    /* If the topenv is corraled and hasn't changed, we can leave it alone */
    if (topend && !topchg) {

        /* ... but unlink non-top symbols from table */
        while (!intop(T)) {
            DROP(T);
        }
        return;
    }

    /* If topenv was mutated, reset corral for next copy low */
    if (topbase && topchg && !(heapbase == heapstart)) {
        heapstart = heap;
        heapsize = HEAPHALF;
        topbase = NULL;
    }

    /* Copy the top environment */
    ccells = 0;
    topend = NULL;
    nil = copyroot(nil);
    EMPTY(T);
    tval = copyroot(tval);
    unval = copyroot(unval);
    topenv = copyroot(topenv);
    topbytes = hp - heapbase;
    topcells = ccells;

    /* If new heap on low side, mark the top's extent and adjust the heap */
    if (CORRAL && !topbase && (heapbase == heapstart)) {

        /* Adjust the top end to be aligned properly */
        if ((ptrdiff_t)hp % HEAPBMOD) {
            hp += HEAPBMOD - ((ptrdiff_t)hp % HEAPBMOD);
        }
        topbase = heapbase;
        topend = (char *)hp;
        topbytes = topend - topbase;

        /* Move heap up past corraled top environment, and stay aligned */
        heapstart = topend;
        heapsize -= (topend - topbase)/2;
        heapsize -= heapsize % HEAPBMOD;
        heapbase = heapstart;
        hp = heapbase;
        other = heapbase + heapsize;
        heapend = heapbase + heapsize;
        heaplim = heapend - GUARDPAGE;
        topchg = 0;
    }
}

/* Garbage collection */
static void gc()
{
    unsigned long nbytes, ncells;
    void (*oldbrk)();

    /* Defer breaks around this code */
    dbrk = 0;
    oldbrk = hookbrk(deferbrk);

    /* Start a new heap */
    newheap();

    /* Copy the roots */

    /* First the eternal, top environment */
    copytop();

    /* Then the rest of the roots */
    ccells = 0;
    S = copyroot(S);
    E = copyroot(E);
    C = copyroot(C);
    D = copyroot(D);

    /* Count bytes and cells */
    nbytes = (topbase && topend) ? topbytes : 0;
    nbytes += hp - heapbase;
    if (maxbytes < nbytes) {
        maxbytes = nbytes;
    }
    ncells = topcells + ccells;
    if (maxcells < ncells) {
        maxcells = ncells;
    }

    /* Restore handler, take deferred breaks */
    hookbrk(oldbrk);
    if (dbrk) {
        (*oldbrk)();
    }
}

/* Time to collect ? */
static int timegc()
{
    return !(hp < heaplim);
}

/* Is it a list, all the way down ? */
static int listp(DCELL *x)
{
    if (NILP(x)) {
        return 1;
    }
    if (ATOMP(x)) {
        return 0;
    }

    return listp(CDR(x));
}

/* Is it a list of lisp data, all the way down ? */
static int listd(DCELL *x)
{
    if (CMNDV(x)) {
        return 0;
    }
    if (NILP(x)) {
        return 1;
    }
    if (ATOMP(x)) {
        return 0;
    }

    return listd(CDR(x));
}

/* List length */
static int listlen(DCELL *x)
{
    int n;
    DCELL *t;

    n = 0;
    t = x;
    while (!NILP(t)) {
        DROP(t);
        ++n;
    }

    return n;
}

/* Initialize */
static void init_data()
{
    /* Initialize heap state */
    heapstart = heap;
    heapsize = HEAPHALF;
    heapbase = heapstart;
    heapend = heapbase + heapsize;
    heaplim = heapend - GUARDPAGE;
    hp = heapbase;
    other = heapbase + heapsize;
    topbase = topend = NULL;
    ccells = gccnt = maxbytes = maxcells = topbytes = topcells = topchg = 0;

    /* First cell is "NIL", the empty list */
    nil = mknil();

    /* Empty symbol table */
    EMPTY(T);

    /* Empty the roots */
    EMPTY(topenv);
    EMPTY(tval);
    EMPTY(unval);
    EMPTY(S);
    EMPTY(E);
    EMPTY(C);
    EMPTY(D);
}

/* Terminate */
static void term_data()
{
    unsigned long symbcnt;

    /* One last GC */
    gc();

    /* Count symbols */
    symbcnt = 0;
    while (!NILP(T)) {
        DROP(T);
        ++symbcnt;
    }

    /* Dump statistics */
    printf("Bytes: %lu/%lu, Cells: %lu/%lu, Symbols: %lu, Collections: %lu\n",
           topbytes, maxbytes, topcells, maxcells, symbcnt, gccnt);
}

/******************************************************************************/
/* Reader                                                                     */
/******************************************************************************/

/* Special characters */
#define CTRL(x) (x - 64)
#define NUL CTRL('@')
#define EOT CTRL('D')
#define BEL CTRL('G')
#define BS CTRL('H')
#define HT CTRL('I')
#define NL CTRL('J')
#define VT CTRL('K')
#define FF CTRL('L')
#define CR CTRL('M')
#define ESC CTRL('[')
#define DEL 127

/* Maximum lengths */
#define MAXSYMLEN 1024
#define TBUFLEN (MAXSYMLEN + 1)
#define MAXLINLEN (heapsize - 3*TBUFLEN)
#define MAXVECLEN 1024
#define INCPATHLEN MAXSYMLEN

/* Line, temporary buffers */
#define TBUF1 (other + 0*TBUFLEN)
#define TBUF2 (other + 1*TBUFLEN)
#define TBUF3 (other + 2*TBUFLEN)
#define LNBUF (other + 3*TBUFLEN)

/* Input stream */
#define NONE ((size_t)-1)
#define MAXFINS 64
#define ISTTY(x) (!finsp && istty(x))
static int finsp, ignore, nlecho, quoted, readdone, verbose;
static size_t escaped, lbri, lblen, notblank;
static FILE *input, *finstk[MAXFINS];

/* Default include subdirectories */
#define SUBDLN strlen("l/xxxxxxxx")
static char sub0[] = "l/lang";
static char sub1[] = "l/clib";
static char sub2[] = "l/ez80";
static char sub3[] = "l/util";
static char *subdir[] = {
    sub0,
    sub1,
    sub2,
    sub3,
    NULL
};

/* Read, REPL environments */
static int done;
static JMP_BUF creadenv, readenv, replenv;

/* Comment, token separator, read macro characters */
static int iscomment(int c)
{
    int r;

    r = (c == ';') || (c == '#');

    return r;
}
static int istoksep(int c)
{
    int r;

    r = isspace(c) || (c == EOF) || (c == EOT) || \
        (c == '(') || (c == ')') || (c == '[') || (c == ']');

    return r;
}
static int isrdmac(int c)
{
    int r;

    r = (c == '\'') || (c == '`') || (c == ',') || (c == '@');

    return r;
}

/* Character and string output that mind verbosity level */
static void vcput(char c)
{
    if (verbose < finsp) {
        return;
    }
    putchar(c);
}
static void vsput(char *s, size_t n)
{
    size_t i;

    i = 0;
    while (i < n) {
        vcput(s[i]);
        ++i;
    }
}

/* Restart REPL from the top */
static void repljmp()
{
    EMPTY(S);
    E = topenv;
    EMPTY(C);
    EMPTY(D);
    LONGJMP(replenv, 1);
}

/* Restart read */
static void readjmp()
{
    LONGJMP(readenv, 1);
}

/* Starting (or re-starting) a read */
static void startread(char *ps)
{
    /* Collect before each new read */
    if (readdone) {
        gc();
        readdone = 0;

        /* Prompt if reading from TTY */
        if (ps && ISTTY(input)) {
            fputs(ps, stdout);
        }
    }
}

/* Ending a read */
static void endread(int neednl)
{
    int c;

    /* Newline after expression read ? */
    if (neednl) {

        /* So we don't echo it later */
        if (!ISTTY(input)) {

            /* Swallow '\n' at line end */ 
            c = '\0';
            while (!(c == '\n')) {
                c = fgetc(input);
                if (c == EOF) {
                    break;
                }
                if (!isspace(c))  {
                    ungetc(c, input);
                    break;
                }
            }
        }

        /* And then print newline, now */
        vcput('\n');
    }

    /* Mark read done, reset line status */
    escaped = NONE;
    ignore = lbri = lblen = notblank = nlecho = quoted = 0;
    readdone = 1;
}

/* End of file action */
static int eof()
{
    /* Not in any included file ? */
    if (!finsp) {
        return 1;
    }

    /* Close the file */
    fclose(input);

    /* Resume previous stream in progress */
    --finsp;
    input = finstk[finsp];

    return 0;
}

/* Error restart, load line buffer with readable quoted symbol string */
static void readerr(char *msg)
{
    char *buf;

    escaped = NONE;
    ignore = lbri = notblank = quoted = 0;
    buf = LNBUF;
    sprintf(buf, "'\"%s\"\n", msg);
    lblen = strlen(buf);
    readjmp();
}
static void errsym(char *msg, char *s, size_t l)
{
    char *buf;
    size_t c, i, n;

    buf = TBUF2;
    i = n = 0;
    l = (l < (MAXSYMLEN - n)) ? l : MAXSYMLEN - n;
    while (i < l) {
        c = s[i++];
        if ((c == '\\') || (c == '"')) {
            buf[n++] = '\\';
            buf[n++] = '\\';
            buf[n++] = '\\';
        }
        buf[n++] = c;
    }
    buf[n] = '\0';

    sprintf(TBUF1, "%s\\\"%s\\\"", msg, buf);
    readerr(TBUF1);
}

/* Break (^C) handlers */
static void brkhand()
{
    while (!eof());
    fputs("^C\n", stdout);
    endread(0);
    repljmp();
}
static void crbrkhand()
{
    LONGJMP(creadenv, 1);
}
   
/* Check for out of memory */
static void memchk()
{
    if (timegc()) {
        readerr("? Out of memory");
    }
}

/* Echo and erase */
static void echo(int c)
{
    if (!ISTTY(input)) {
        switch (c) {
            case '\n' :
                vcput('\n');
                break;
            case DEL :
                vsput("^?", strlen("^?"));
                break;
            default :
                if (c < 32) {
                    vcput('^');
                    c += 64;
                }
                vcput(c);
                break;
        }
    } else {
        switch (c) {
            case EOT :
                break;
            case '\n' :
                putchar('\n');
                break;
            case '\b' :
            case DEL :
                break;
            default :
                if (c < 32) {
                    putchar('^');
                    c += 64;
                }
                putchar(c);
                break;
        }
    }
    nlecho = (c == '\n');
}
static void cerase()
{
    int c;

    /* Rub out character on screen */
    if (!ISTTY(input)) {
        return;
    }
    c = LNBUF[lblen];
    switch (c) {
        case EOT :
        case '\b' :
        case '\n' :
        case DEL :
            break;
        default :
            fputs("\b \b", stdout);
            if (c < 32) {
                fputs("\b \b", stdout);
            }
            break;
    }
}

/* Update the comment state in 'ignore' */
static int incomment()
{
    int c;
    size_t i;

    /* Escape expired ? */
    i = lbri - 1;
    if (!(escaped == NONE)) {
        if (1 < (i - escaped)) {
            escaped = NONE;
        }
    }

    c = LNBUF[i];
    switch (c) {

        case '\n':
            /* Comment ending ? */
            if (!quoted && (escaped == NONE)) {
                ignore = 0;
            }
            break;

        case '\\' :
            /* Escaped ? */
            if (!ignore && (escaped == NONE)) {
                escaped = i;
            }
            break;

        case '"' :
            /* Quoted ? */
            if (!ignore && (escaped == NONE)) {
                quoted = !quoted;
            }
            break;

        default :
            /* Comment beginning ? */
            if (iscomment(c)) {
                if (!quoted && (escaped == NONE)) {
                    ignore = 1;
                }
            }
            break;
    }

    return ignore;
}

/* Edit the line buffer, or special character actions */
static void edit(int c)
{
    switch (c) {

        case EOT :
            /* Kill line */
            if (!lblen) {
                return;
            }
            endread(!nlecho);
            break;

        case '\b' :
        case DEL :
            /* Erase characters on screen, back up, restart */
            if (lblen) {
                --lblen;
                cerase();
                escaped = NONE;
                ignore = lbri = notblank = quoted = 0;
            } else {
                endread(!nlecho);
            }
            break;

        default :
            return;
    }

    /* Restart read */
    readjmp();
}

/* Linebuffer/stream access */
static void myungetc(char c)
{
    int i;

    /* Not a real character */
    if (c == EOF) {
        return;
    }

    /* Push character back */
    if (lbri) {

        /* Undo or reinstate escape */
        --lbri;
        if (!(escaped == NONE)) {
            if (escaped == lbri) {
                escaped = NONE;
            }
        } else {
            if (lbri && (LNBUF[lbri - 1] == '\\')) {
                escaped = lbri - 1;
            }
        }

        /* Toggle quote, if not in the shadow of an escape */
        if (LNBUF[lbri] == '"') {
            if ((escaped == NONE) || (1 < (lbri - escaped))) {
                quoted = !quoted;
            }
        }
        LNBUF[lbri] = c;
    } else {

        /* Push new character onto stream */
        if (lblen < MAXSYMLEN) {
            ++lblen;
            i = lblen - 1;
            while (i) {
                LNBUF[i] = LNBUF[i - 1];
                --i;
            }
            LNBUF[0] = c;
        }
    }
}
static int mygetc()
{
    int c;

    /* Get character from line buffer or input stream */
    while (1) {
        if (lbri < lblen) {
            c = LNBUF[lbri++];
        } else {
            c = fgetc(input);

            /* End of file ? */
            if (c == EOF) {
                if (lblen) {
                    readerr("? Incomplete expression at end of file");
                }
                if (eof()) {
                    done = 1;
                    repljmp();
                } else {
                    /* Simulate blank line at end of file */
                    nlecho = 1;
                    readerr("");
                }
            }

            /* Carriage returns become newlines */
            if (c == '\r') {
                c = '\n';
            }
        
            /* Echo, edit, store */
            echo(c);
            edit(c);
            if (!(lblen < MAXLINLEN)) {
                readerr("? Line too long");
            }
            LNBUF[lblen++] = c;
            lbri = lblen;
        }

        /* Not in a comment ? */
        if (!incomment()) {

            /* On blank line, skip to next */
            if (isspace(c) && !notblank) {
                endread(!nlecho);
                readjmp();
            }

            /* Return this character */
            notblank = 1;
            break;
        }
    }

    return c;
}

/* Read macros */
static DCELL *read1();
static DCELL *readm()
{
    char *s;
    int c, n;

    /* Quote, backquote, unquote, etc. shorthand */
    c = mygetc();
    s = NULL;
    if (c == '\'') {
        s = "quote";
    }
    if (c == '`') {
        s = "backquote";
    }
    if (c == ',') {
        n = mygetc();
        if (n == '@') {
            s = "unquote-splicing";
            c = n;
        } else {
            s = "unquote";
            myungetc(n);
        }
    }
    if (s) {
        n = mygetc();
        myungetc(n);
        if (isspace(n) || (n == ')') || (n == ']')) {
            sprintf(TBUF1, "? Ambiguous syntax after \\\"%c\\\"", (char)c);
            readerr(TBUF1);
        }
        return mkpair(MKSYMB(s), mkpair(read1(), nil)); 
    }

    myungetc(c);

    return NULL;
}

/* Numbers */
static int isdig(int h, int c)
{
    return h ? isxdigit(c) : isdigit(c);
}
static DCELL *readn()
{
    int c, d, h, i, l, n, t;
    ptrdiff_t inum;
    double rnum;
    DCELL *v;

    /* Get a token */
    t = 0;
    while (1) {
        c = mygetc();
        if (istoksep(c)) {
            myungetc(c);
            break;
        }
        if (!(t < MAXSYMLEN)) {
            readerr("? Token too long");
        }
        TBUF1[t] = c;
        ++t;
    }
    TBUF1[t] = '\0';

    /* Get a numeric string from the token */
    d = h = i = l = 0;
    if (!strncmp(TBUF1, "0x", strlen("0x"))) {
        h = 1;
        l += strlen("0x");
    }
    c = n = '\0';
    while (l < t) {
        c = TBUF1[l + 0];
        n = TBUF1[l + 1];
        if (!isdig(h, c)) {
            if ((c == '+') || (c == '-')) {
                if (i || (!isdig(h, n) && !(n == '.'))) {
                    break;
                }
            } else {
                if (c == '.') {
                    if (!isdig(h, n) && n) {
                        break;
                    }
                    ++d;
                } else {
                    break;
                }
            }
        }
        TBUF2[i] = c;
        ++i;
        ++l;
    }
    TBUF2[i] = '\0';

    /* More in the token after a numeric string ? */   
    if (!(d < 2) || (l && (l < t))) {
        errsym("? Extra input after number: ", TBUF1, t);
    }

    /* Convert the string */
    if (!d) {

        /* Try integer first */
        i = sscanf(TBUF2, h ? AHEXFMT : AINTFMT, &inum);
        if (i == 1) {
            v = mkintn(inum);
            memchk();
            return v;
        }
    }     

    /* Try real number */
    i = sscanf(TBUF2, "%lg", &rnum);
    if (i == 1) {
        v = mknumb(rnum);
        memchk();
        return v;
    }

    /* If it doesn't go, put it back */
    i = strlen(TBUF1);
    while (i) {
        myungetc(TBUF1[i - 1]);
        --i;
    }

    return NULL;
}

/* Pairs */
static DCELL *readp(int l)
{
    int c, d, oc;
    DCELL *x;

    oc = (l & 1) ? '[' : '(';
    c = '\0';
    if (!(l & 2)) {
        c = mygetc();
        if (!(c == oc)) {
            myungetc(c);
        }
    }
    if ((l & 2) || (c == oc)) {

        /* End of list ? */
        while (isspace(c = mygetc()));
        if ((c == ')') || (c == ']')) {
            if (((l & 1) && (c == ')')) || (!(l & 1) && (c == ']'))) {
                sprintf(TBUF1, "? Unexpected: \\\"%c\\\"", c);
                readerr(TBUF1);
            }
            return nil;
        } else {
            myungetc(c);
        }

        /* Get car */
        x = read1();

        /* If not in vector, look for ". " */
        d = 0;
        if (!(l & 1)) {
            while (isspace(c = mygetc()));
            if (c == '.') {
                c = mygetc();
                myungetc(c);
                if (isspace(c)) {
                    d = 1;
                } else {
                    myungetc('.');
                }
            } else {
                myungetc(c);
            }
        }
        if (!d) {

            /* Rest of list */
            x = mkpair(x, readp(l | 2));
        } else {

            /* Second element of dotted pair */
            x = mkpair(x, read1());

            /* Grab ')' */
            while (isspace(c = mygetc()));
            if (!(c == ')')) {
                readerr("? Improper dotted pair");
            }
        }

        memchk();
        return x;
    }

    return NULL;
}

/* Symbol */
static DCELL *reads()
{
    int b, c, e, k, n, q, t;
    DCELL *x;

    /* Symbol */
    b = 10;
    e = k = n = q = 0;
    while (1) {

        /* Too long */
        if (!(n < MAXSYMLEN)) {
            readerr("? Symbol too long");
        }

        /* Get character */
        c = mygetc();

        /* Translate "^" control characters */
        if (k) {
            if (!(c < '@') && !('_' < c)) {
                c -= '@';
            } else {
                /* Or just put '^' in symbol */
                TBUF1[n] = '^';
                ++n;
            }
        }

        /* Escape ? */
        if (!e && (c == '\\')) {
            e = 1;
            continue;
        }

        /* Quote ? */
        if (!e && (c == '"')) {
            q = !q;
            continue;
        }

        /* Control character "^" next ? */
        if (!e && !k && (c == '^')) {
            k = 1;
            continue;
        }

        /* Read macro character ? */
        if (!e && !q && isrdmac(c)) {
            sprintf(TBUF1,
                    "? \\\"%c\\\" must be escaped or quoted in symbols", c);
            readerr(TBUF1);
        }

        /* Translate escaped characters */
        if (e) {
            switch (c) {
                case 'a' :
                    c = BEL;
                    break;
                case 'b' :
                    c = BS;
                    break;
                case 'e' :
                    c = ESC;
                    break;
                case 'f' :
                    c = FF;
                    break;
                case 'n' :
                    c = NL;
                    break;
                case 'r' :
                    c = CR;
                    break;
                case 't' :
                    c = HT;
                    break;
                case 'v' :
                    c = VT;
                    break;
                case 'x' :
                    /* Hex character constant */
                    c = 0;
                    t = EOF;
                    while ((c < 256) && isxdigit(t = mygetc())) {
                        if (isalpha(t)) {
                            t = 10 + (toupper(t) - 'A');
                        } else {
                            t -= '0';
                        }
                        c = c*16 + t;
                    }
                    if (!(c < 256)) {
                        e = -1;
                    }
                    myungetc(t);
                    break;
                case '0' :
                    /* Octal character constant */
                    b = 8;
                case '1' :
                case '2' :
                case '3' :
                case '4' :
                case '5' :
                case '6' :
                case '7' :
                case '8' :
                case '9' :
                    /* Decimal character constant */
                    c -= '0';
                    t = EOF;
                    while (c < 256) {
                        t = mygetc();
                        if (!(t < '0') && (t < ('0' + b))) {
                            c = c*b + (t - '0');
                        } else {
                            break;
                        }
                    }
                    if (!(c < 256) || (isdigit(t) && !(t < ('0' + b)))) {
                        e = -1;
                    }
                    myungetc(t);
                    break;
            }
            if (e < 0) {
                readerr("? Improper character constant");
                break;
            }
        }

        /* End of symbol token ? */
        if (!e && !q && istoksep(c)) {

            /* EOT is special */
            if (c == EOT) {
                TBUF1[n] = c;
                ++n;
            } else {
                myungetc(c);
            }
            break;
        }

        /* Store character */
        TBUF1[n] = c;
        ++n;
        e = k = 0;
    }
    TBUF1[n] = '\0';

    x = mksymb(TBUF1, n);
    memchk();

    return x;
}

/* Vectors */
static DCELL *readv()
{
    int n;
    DCELL *x;

    x = readp(1);
    if (x) {
        n = listlen(x);
        if (MAXVECLEN < n) {
            sprintf(TBUF1, "? Length (%d) exceeds maximum (%d) for vectors",
                    n, MAXVECLEN);
            readerr(TBUF1);
        }

        x = mkvect(x);
        memchk();

        return x;
    }

    return NULL;
}

/* Read an expression from the input stream */
static DCELL *read1()
{
    int c;
    DCELL *x;

    /* Skip white space */
    while (isspace(c = mygetc()));
    myungetc(c);

    /* Read macro ? */
    x = readm();

    /* Vector ? */
    if (!x) {
        x = readv();
    }

    /* Pair ? */
    if (!x) {
        x = readp(0);
    }

    /* Number ? */
    if (!x) {
        x = readn();
    }

    /* Symbol ? */
    if (!x) {
        x = reads();
    }

    return x;
}
static DCELL *exread(char *ps)
{
    int c;
    DCELL *x;

    /* Get started */
    SETJMP(readenv);
    startread(ps);

    /* Read */
    x = read1();

    /* Unparseable, or something extra ? */
    while (lbri < lblen) {
        c = mygetc();
        if (!isspace(c)) {
            myungetc(c);
            break;
        }
    }
    if (!x || (lbri < lblen)) {
        errsym("? Unexpected: ", &LNBUF[lbri], lblen - lbri);
    }

    /* Reset for next read */
    endread(!nlecho);

    return x;
}
static void read()
{
    DCELL *x;

    /* Top continuation */
    PUSH(D, mkcont(S, E, C));

    /* Read expression */
    x = exread("> ");

    /* End of input symbol ? */
    if (SYMBP(x) && ((SYMBN(x))[0] == EOT)) {
        if (ISTTY(input)) {
            done = 1;
            repljmp();
        } else {
            eof();
            x = CONS(MKSYMB("quote"), CONS(MKSYMB(""), nil));
        }
    }

    /* Push expression */
    PUSH(S, x);
}

/* Find path to include file */
static void incpath()
{
    char *cp, *filename, *path, *defpath, *savpath;
    int i;
    FILE *f;

    /* Path output to TBUF1, filename on entry in TBUF2 */
    path = TBUF1;
    filename = TBUF2;

    /* Copy filename */
    if ((INCPATHLEN - 1) < strlen(filename)) {
        path[0] = '\0';
        return;
    }
    strcpy(path, filename);

    /* If it starts with "/", it's a full path, we're done */
    if (path[0] == '/') {
        return;
    }

    /* If it doesn't start with "<", ... */
    if (!(path[0] == '<')) {

        /* ... try "." first ... */
        if ((f = fopen(path, "r"))) {
            fclose(f);
            return;
        }

        /* ... then put it on the default path */
        if ((INCPATHLEN - 3) < strlen(filename)) {
            path[0] = '\0';
            return;
        }
        strcpy(path, "<");
        strcat(path, filename);
        strcat(path, ">");
    }

    /* Default include path specified ? */
    defpath = TBUF2;
    savpath = TBUF3;
    if ((path[0] == '<') && (path[strlen(path) - 1] == '>')) {

        /* Save specified part of path */
        strncpy(savpath, path + 1, strlen(path) - 2);
        savpath[strlen(path) - 2] = '\0';

        /* Get default include path root */
        i = getdefinc(defpath, INCPATHLEN);
        if ((i < 0) || !(i < (INCPATHLEN - strlen(savpath) - SUBDLN))) {
            path[0] = '\0';
            return;
        }

        /* Try all the subdirectories off the root */
        i = 0;
        while ((cp = subdir[i])) {
            strcpy(path, defpath);
            strcat(path, cp);
            strcat(path, "/");
            strcat(path, savpath);
            if ((f = fopen(path, "r"))) {
                fclose(f);
                return;
            }
            ++i;
        }
    }

    path[0] = '\0';
}

/* Open include file */
static int include(char *filename)
{
    char *path;

    /* Save input stream in progress */
    if (!(finsp < MAXFINS)) {
        return 1;
    }
    finstk[finsp] = input;
    ++finsp;

    /* Open include file */
    input = NULL;
    if (strlen(filename)) {
        path = TBUF1;
        strcpy(TBUF2, filename);
        incpath();
        if (strlen(path)) {
            input = fopen(path, "r");
        }
    }

    /* Problem ? Stay in current stream */
    if (!input) {
        --finsp;
        input = finstk[finsp];
        return -1;
    }

    return 0;
}

/* Initialize */
static void init_read()
{
    escaped = NONE;
    ignore = lbri = lblen = notblank = nlecho = quoted = 0;
    readdone = 1;
    input = stdin;
    finsp = verbose = 0;
    setvbuf(stdout, NULL, _IONBF, 0);

    /* Set TTY in raw mode */
    if (istty(input)) {
        ttyopen();
    }

    /* Open start-up file */
    include(STARTUP);
}

/* Terminate */
static void term_read()
{
    /* Close open input files */
    while (!eof());

    /* Reset TTY */
    ttyclose();
}

/******************************************************************************/
/* Printer                                                                    */
/******************************************************************************/

/* Primitive function definition */
typedef struct pfunc_struct {
    char pname[16];
    char cname[16];
    int narg;
    void (*fcode)();
} PFUNC;

/* Output buffer */
#define OBUF LNBUF
#define OEND (&OBUF[oblen])
#define OPUTC(c) OBUF[oblen++] = c
#define OPUTS(s) { strcpy(OEND, s); oblen += strlen(s); }
#define OSPRINTF(f, d) { sprintf(OEND, f, d); oblen += strlen(OEND); }
static size_t oblen;

/* Print a string, verbatim */
static void snprt(DCELL *x)
{
    char *s;
    size_t i, n;

    s = SYMBN(x);
    n = SYMBL(x);
    i = 0;
    while (i < n) {
        OPUTC(s[i]);
        ++i;
    }
}

/* Print a string with escaped characters, readably */
static void escsprt(DCELL *x)
{
    char *s;
    int c, q;
    size_t i, n;

    /* Do we want to enclose in quotes ? */
    s = SYMBN(x);
    n = SYMBL(x);
    q = !n;
    i = 0;
    while ((i < n) && !q) {
        c = s[i];
        if ((c < 32) || (c == '\\') || (c == '"') || \
            istoksep(c) || isrdmac(c) || iscomment(c)) {
            q = 1;
        }
        ++i;
    }

    /* Open quote */
    if (q) {
        OPUTC('"');
    }

    /* Print characters, escaping as necessary */
    i = 0;
    while (i < n) {
        c = s[i];
        switch (c) {
            case BEL :
                OPUTS("\\a");
                break;
            case BS :
                OPUTS("\\b");
                break;
            case ESC :
                OPUTS("\\e");
                break;
            case FF :
                OPUTS("\\f");
                break;
            case CR :
                OPUTS("\\r");
                break;
            case HT :
                OPUTS("\\t");
                break;
            case NL :
                OPUTS("\\n");
                break;
            case VT :
                OPUTS("\\v");
                break;
            case '\\' :
            case '"' :
                OPUTC('\\');
                OPUTC(c);
                break;
            default :
                if (!(ESC < c)) {
                    OPUTC('^');
                    OPUTC('@' + c);
                } else {
                    OPUTC(c);
                }
                break;
        }
        ++i;
    }

    /* Close quote */
    if (q) {
        OPUTC('"');
    }
}

/* Print an expression to the output stream */
static void pcmdprint(DCELL *c);
static void vcmdprint(DCELL *c);
static PFUNC *ispcmd(DCELL *c);
static void print1(DCELL *x, int l)
{
    double d;
    DCELL *t;

    /* Command ? */
    if (CMNDV(x)) {
        if (ispcmd(x)) {
            pcmdprint(x);
        } else {
            vcmdprint(x);
        }
        return;
    }

    /* Integer ? */
    if (INTNP(x)) {
        OSPRINTF(AINTFMT, INTNF(x));
        return;
    }

    /* Real number ? */
    if (NUMBP(x)) {
        d = NUMBF(x);
        OSPRINTF("%lg", d);
        if (d == (double)((ptrdiff_t)d)) {
            OPUTS(".0");
        }
        return;
    }

    /* Symbol ? */
    if (SYMBP(x)) {
        if (!l) {
            snprt(x);
        } else{
            escsprt(x);
        }
        return;
    }

    /* Address or continuation ? */
    if (ADDRP(x) || CONTP(x)) {
        OSPRINTF(ADDRFMT, (size_t)x);
        return;
    }

    /* Vector ? */
    if (VECTP(x)) {
        print1(VECTF(x), 2);
        return;
    }

    /* Pair or () ? */
    if (NILP(x) || PAIRP(x)) {
        OPUTC((l == 2) ? '[' : '(');
        t = x;
        while (PAIRP(t)) {
            if (!(t == x)) {
                OPUTC(' ');
            }
            print1(CAR(t), VECTP(CAR(t)) ? 2 : 1);
            DROP(t);
        }
        if (!NILP(t)) {
            OPUTS(" . ");
            print1(t, l);
        }
        OPUTC((l == 2) ? ']' : ')');
        return;
    }

    /* Unknown */
    OPUTS("???");
}
static void print()
{
    DCELL *x;

    POP(S, x);
    oblen = 0;
    print1(x, 0);
    vsput(OBUF, oblen);
    if (oblen) {
        if (!(OBUF[oblen - 1] == '\n')) {
            vcput('\n');
        }
    }
}
static void exprint(DCELL *x)
{
    size_t i;

    oblen = 0;
    print1(x, 0);
    i = 0;
    while (i < oblen) {
        putchar(OBUF[i]);
        ++i;
    }
}

/******************************************************************************/
/* Evaluator                                                                  */
/******************************************************************************/

/* Core virtual machine commands */
#define C_APPLY    (DCELL *)c_apply
#define C_ARGC     (DCELL *)c_argc
#define C_CONTINUE (DCELL *)c_continue
#define C_DEFINE   (DCELL *)c_define
#define C_END      (DCELL *)c_end
#define C_EVALC    (DCELL *)c_evalc
#define C_GET00    (DCELL *)c_get00
#define C_GET10    (DCELL *)c_get10
#define C_GET20    (DCELL *)c_get20
#define C_GETB     (DCELL *)c_getb
#define C_GETL     (DCELL *)c_getl
#define C_JUMPC    (DCELL *)c_jumpc
#define C_JUMPM    (DCELL *)c_jumpm
#define C_LAMBDA   (DCELL *)c_lambda
#define C_LOAD     (DCELL *)c_load
#define C_LOADC    (DCELL *)c_loadc
#define C_MACRO    (DCELL *)c_macro
#define C_MAKECC   (DCELL *)c_makecc
#define C_QUOTE    (DCELL *)c_quote
#define C_SELECT   (DCELL *)c_select
#define C_SETB     (DCELL *)c_setb
#define C_SETL     (DCELL *)c_setl

/* Components of function applications */
#define FUNC(x) (CAR(x))
#define ARGS(x) (CDR(x))
#define ARG1(x) (CAR(x))
#define ARG2(x) (CAR(CDR(x)))
#define ARG3(x) (CAR(CDR(CDR(x))))

/* Special forms */
#define CALLCC "call/cc"
#define DEFINE "define"
#define IF "if"
#define LAMBDA "lambda"
#define MACRO "macro"
#define PROGN "progn"
#define QUOTE "quote"
#define SETQ "setq"

/* Magic variables */
#define DO_NOT_PATCH "*don't-patch*"
#define ERROR_CONTINUE "*error-continue*"
#define DO_NOT_FORGET "*don't-forget*"
#define TRACED_OBJECTS "*traced-objects*"

/* Debugger primitive, repl */
#define DEBUGGER "debugger"
#define DREPL "repl"

/* Truth */
#define TVAL "t"

/* Error message buffer */
#define EBUF TBUF3

/* Maximum number of commands in in-lined expression */
#define MAXINLINE 19

/* Macro ? */
#define ISMACRO(v) (PAIRD(v) && NILP(CAR(v)))

/* Hard (unbound, materialized) primitive ? */
#define ISHARDP(idx, lev) ((ptrdiff_t)lev < 0 ? (PFUNC *)idx : NULL)

/* Primitive function command fetch */
#define PFCMDF(v) (NILP(CDR(v)) ? CAR(v) : CAR(CDR(CDR(v))))

/* Special form ? */
#define ISFORM(f) \
    (EQNAM(f, CALLCC) || EQNAM(f, DEFINE) || EQNAM(f, IF) || \
     EQNAM(f, LAMBDA) || EQNAM(f, MACRO) || EQNAM(f, PROGN) || \
     EQNAM(f, QUOTE) || EQNAM(f, SETQ))

/* Dispatch a command from the list */
#define CALLC() { DCELL *_f; POP(C, _f); ((PFV)_f)(); }
typedef void (*PFV)();

/* Gensym counter */
static unsigned long gsymc;

/* 'forget' enabled option */
static int forget;

/* Break (^C) handler */
char *ebrkstr;
int dontbrk, ebrk;

/* Eval command loop return environments */
static JMP_BUF evalenv;
static JMP_BUF cloopenv;

/* Eval error handlers */
static void evalerr(char *msg)
{
    while (!eof());
    if (!(msg == EBUF)) {
        strcpy(EBUF, msg);
    }
    dontbrk = ebrk = 0;
    LONGJMP(evalenv, 1);
}
static void ebrkhand()
{
    ebrk = 1;
}
static void efpehand()
{
    evalerr("? Arithmetic exception");
}
static void ebrkjmp()
{
    evalerr(ebrkstr);
}

/* Collect if it's time */
static void evalgc()
{
    /* Did a break happen ? */
    if (ebrk && !dontbrk) {
        ebrkjmp();
    }

    /* Time to collect ? */
    if (timegc()) {

        /* Out of memory ? */
        gc();
        if (timegc()) {
            evalerr("? Out of memory");
        }
    }
}

/* Extend the bottom level of the specified environment */
static void eappend(DCELL *e, DCELL *s, DCELL *v)
{
    DCELL *a, *b, *p;

    b = CONS(CONS(s, v), nil);
    p = NULL;
    a = CAR(e);
    while (!NILP(a)) {
        p = a;
        a = CDR(a);
    }
    if (!p) {
        TOPCHG(e);
        CAR(e) = b;
    } else {
        TOPCHG(p);
        CDR(p) = b;
    }
}

/* Make a built-in primitive routine */
static DCELL *mkprim(PFUNC *pf)
{
    DCELL *f;

    EMPTY(f);
    PUSH(f, (DCELL *)pf->fcode);
    if (!(pf->narg < 0)) {
        PUSH(f, mkintn(pf->narg));
        PUSH(f, C_ARGC);
    }

    return f;
}

/* Auto-load lambda or macro definitions from files */
static DCELL *cload(DCELL *p, DCELL *x);
static DCELL *esearch(DCELL *s, size_t *idx, size_t *lev);
static int autoload(DCELL *s)
{
    char *fnam, *path;
    size_t idx, lev;
    DCELL *b, *q, *v;

    /* These can't be auto-loaded, they're needed for auto-loading */
    if (EQNAM(s, "backquote") || EQNAM(s, "loadapply")) {
        return 0;
    }

    /* Can we find the right file ? */
    path = TBUF1;
    fnam = TBUF2;
    strcpy(fnam, SYMBN(s));
    strcat(fnam, ".l");
    incpath();
    if (strlen(path)) {

        /* Do by hand what 'import' macro does */

        /* Don't do this if the necessary functions are missing */
        if (!esearch(MKSYMB("backquote"), &idx, &lev) ||
            !esearch(MKSYMB("loadapply"), &idx, &lev)) {
            return 0;
        }

        /* Make the stub and bind the symbol to it */
        EMPTY(q);
        PUSH(q, MKSYMB("args"));
        PUSH(q, MKSYMB("unquote-splicing"));
        EMPTY(b);
        PUSH(b, q);
        PUSH(b, s);
        PUSH(b, MKSYMB(path));
        PUSH(b, MKSYMB("loadapply"));
        EMPTY(q);
        PUSH(q, b);
        PUSH(q, MKSYMB("backquote"));
        EMPTY(v);
        v = cload(v, q);
        PUSH(v, MKSYMB("args"));
        PUSH(v, nil);
        eappend(topenv, s, v);

        return 1;
    }

    return 0;
}

/* Forget a top-level binding */
static int eforget(DCELL *s)
{
    size_t idx, lev;
    DCELL *a, *b, *p;

    /* Forget disabled ? */
    if (!forget) {
        return 0;
    }
    if ((b = esearch(MKSYMB(DO_NOT_FORGET), &idx, &lev))) {
        if (!NILP(CDR(b))) {
            return 0;
        }
    }

    /* Is this symbol being traced ? */
    if ((b = esearch(MKSYMB(TRACED_OBJECTS), &idx, &lev))) {
        a = CDR(b);
        while (!NILP(a)) {
            b = CAR(a);
            if (CAR(b) == s) {
                break;
            }
            DROP(a);
        }
        if (!NILP(a)) {
            return 0;
        }
    }

    /* Search the top a-list */
    a = CAR(topenv);
    p = NULL;
    while (!NILP(a)) {

        /* Found ? */
        b = CAR(a);
        if (CAR(b) == s) {

            /* Remove binding from a-list */
            if (!p) {
                DROP(a);
                TOPCHG(topenv);
                CAR(topenv) = a;
            } else {
                TOPCHG(p);
                CDR(p) = CDR(a);
            }
            return 1;
        }

        p = a;
        DROP(a);
    }

    return 0;
}

/* Set list of bindings in top environment after a mark */
static DCELL *setetop(DCELL *s, DCELL *t)
{
    DCELL *a, *b, *d;

    /* Search the top a-list */
    a = CAR(topenv);
    while (!NILP(a)) {

        /* Found ? */
        b = CAR(a);
        if (CAR(b) == s) {

            /* Get and replace tail of list */
            TOPCHG(a);
            d = mkaddr(CDR(a));
            if (NILP(t) || ADDRP(t)) {
                b = t;
                if (ADDRP(b)) {
                    b = ADDRF(b);
                }
                CDR(a) = b;
            }

            return d;
        }

        DROP(a);
    }

    return nil;
}

/* Special symbols aren't in files */
static int special(DCELL *s)
{
    char fc;

    fc = *(SYMBN(s));

    return (fc == '*');
}

/* Search the current environment for a binding */
static PFUNC *ispnam(DCELL *x);
static DCELL *esearch(DCELL *s, size_t *pidx, size_t *plev)
{
    DCELL *a, *b, *la;
    PFUNC *pf;
    size_t i, l;

    /* Search the list of a-lists */
    b = nil;
    i = l = 0;
    la = E;
    while (!NILP(la)) {

        /* Search this a-list */
        i = 0;
        a = CAR(la);
        while (!NILP(a)) {

            /* Found ? */
            b = CAR(a);
            if (CAR(b) == s) {
                break;
            }

            DROP(a);
            ++i;
        }
        if (!NILP(a)) {
            break;
        }

        DROP(la);
        ++l;
    }

    /* If we broke out, we found it */
    if (!NILP(la)) {
        *plev = l;
        *pidx = i;
        return b;
    }

    /* Primitive ? */
    pf = ispnam(s);
    if (pf) {
        b = CONS(s, mkprim(pf));
        *plev = (size_t)-1;
        *pidx = (size_t)pf;
        return b;
    }

    /* Not found so far, try autoload */
    if (!special(s) && autoload(s)) {
        return esearch(s, pidx, plev);
    }

    /* Not found at all */
    *plev = *pidx = (size_t)-1;
    return NULL;
}

/* Reverse a list */
static void revlist(DCELL *fwd, DCELL **prev)
{
    if (NILP(fwd)) {
        return;
    }
    *prev = CONS(CAR(fwd), *prev);
    revlist(CDR(fwd), prev);
}

/* Add progn to body unless body is a single expression */
static DCELL *addprogn(DCELL *x)
{
    DCELL *a, *b, *t;

    a = CAR(x);
    b = CDR(x);
    if (listlen(b) == 1) {
        return x;
    }

    PUSH(b, MKSYMB(PROGN));
    EMPTY(t);
    PUSH(t, b);
    PUSH(t, a);

    return t;
}

/* Push C_EVALC or C_JUMPC */
static DCELL *evaljmp(DCELL *x)
{
    DCELL *r;

    r = x;
    PUSH(r, NILP(CDR(r)) ? C_JUMPC : C_EVALC);

    return r;
}

/* Compile a command list load or an inline list for an expression */
static DCELL *cload(DCELL *p, DCELL *x)
{
    DCELL *l, *r;

    /* Link a (C_LOADC command) onto the front of the existing list of */
    /* commands. This C_LOADC command contains the address of the list */
    /* returned by this routine - CAR(list) = (C_LOADC command). When */
    /* executed, C_LOADC will patch CAR(list), replacing the reference */
    /* to the C_LOADC command with one to the command list expanded from */
    /* the expression. The list being patched is part of a lambda, or */
    /* a C_EVALC, C_JUMPC, C_JUMPM, or C_SELECT command. */
    r = p;
    PUSH(r, NULL);             /* 1. This is where ... */
    EMPTY(l);
    PUSH(l, x);                /* expression to expand */
    PUSH(l, NULL);             /* 2. This is where ... */
    PUSH(l, C_LOADC);
    CAR(CDR(l)) = mkaddr(r);   /* 2. ... this goes */
    CAR(r) = l;                /* 1. ... this goes */

    return r;
}
static DCELL *exprcmnd(DCELL *p, DCELL *x);
static DCELL *eload(DCELL *p, DCELL *x)
{
    int i;
    size_t idx, lev;
    DCELL *b, *f, *r;
    PFUNC *pf;

    /* Inline atoms, some special forms, primitive applications, and macros */
    i = ATOMP(x);
    if (!i && !CMNDV(x)) {
        f = CAR(x);
        if (SYMBP(f)) {
            i = EQNAM(f, QUOTE) || \
                (EQNAM(f, PROGN) && (NILP(CDR(x)) || NILP(CDR(CDR(x)))));
            if (!i && !ISFORM(f)) {
                if ((b = esearch(f, &idx, &lev))) {
                    if ((pf = ISHARDP(idx, lev))) { 
                        i = !(pf->narg < 0);
                    } else {
                        i = ISMACRO(CDR(b));
                    }
                }
            }
        }
    }

    /* Inline by putting commands on front of existing list */
    if (i) {

        /* But not if list is at max commands before GC check */
        f = p;
        i = 0;
        while (!NILP(f)) {
            if (CMNDP(f)) {
                ++i;

                /* GC check is done by these commands */
                if ((CAR(f) == C_EVALC) || (CAR(f) == C_JUMPC) || \
                    (CAR(f) == C_SELECT)) {
                    break;
                }
            }
            DROP(f);
        }
        if (!(MAXINLINE < i)) {
            r = exprcmnd(p, x);
            return r;
        }
    }

    /* Create load and eval/jump commands for a subexpression */
    r = cload(p, x);
    r = evaljmp(r);

    return r;
}

/* Apply macro transformation and load into text */
static DCELL *mload(DCELL *p, DCELL *m, DCELL *a)
{
    DCELL *f, *h, *l, *r;

    /* Patch result of the application into text, evaluate */
    EMPTY(l);
    PUSH(l, NULL);
    h = l;
    PUSH(l, C_LOAD);

    /* Apply the transforming function to the quoted arguments */
    EMPTY(f);
    PUSH(f, C_APPLY);
    PUSH(f, m);
    PUSH(f, C_QUOTE);

    /* Quote all the arguments */
    while (!NILP(a)) {
        PUSH(f, CAR(a));
        PUSH(f, C_QUOTE);
        DROP(a);
    }

    /* Evaluate transformation subexpression */
    PUSH(l, f);
    PUSH(l, C_EVALC);

    /* Make a jump command to hold all of it */
    r = p;
    PUSH(r, l);
    PUSH(r, C_JUMPM);

    /* Point LOAD command at jump command */
    CAR(h) = mkaddr(r);

    return r;
}

/* Is this level the top ? */
static int istop(size_t lev)
{
    size_t l;
    DCELL *la;

    la = E;
    l = 0;
    while (l < lev) {
        DROP(la);
        ++l;
    }

    return (la == topenv);
}

/* Expand an expression into VM commands */
static DCELL *exprcmnd(DCELL *r, DCELL *x)
{
    int d, n;
    size_t idx, lev;
    DCELL *a, *b, *f, *t;
    PFUNC *pf;

    /* Symbol ? */
    if (SYMBP(x)) {

        /* Search environment */
        b = esearch(x, &idx, &lev);

        /* Variable not bound or assigned ? */
        if (!b || (CDR(b) == unval)) {
            sprintf(EBUF,
                "? Un%s variable: \"%s\"", b ? "assigned" : "bound", SYMBN(x));
            evalerr(EBUF);
        }

        /* Hard primitive ? */
        if (ISHARDP(idx, lev)) {
            PUSH(r, CDR(b));
            PUSH(r, C_QUOTE);
            return r;
        }

        /* Top level variable ? */
        if (istop(lev)) {
            PUSH(r, mkaddr(b));
            PUSH(r, C_GETB);
            return r;
        }

        /* Bound at 0, 1, or 2 position at this level ? */
        if (!lev) {
            switch (idx) {
                case 0 :
                    PUSH(r, C_GET00);
                    return r;
                case 1 :
                    PUSH(r, C_GET10);
                    return r;
                case 2 :
                    PUSH(r, C_GET20);
                    return r;
            }
        }

        /* Fetch from index and level */
        PUSH(r, mkintn((ptrdiff_t)lev));
        PUSH(r, mkintn((ptrdiff_t)idx));
        PUSH(r, C_GETL);
        return r;
    }

    /* Any atom other than a symbol is a 'constant' */
    if (ATOMP(x)) {
        PUSH(r, x);
        PUSH(r, C_QUOTE);
        return r;
    }

    /* Non-lisp-data-list pairs are an error */
    if (!listd(x)) {
        evalerr("? Expression with no value");
    }

    /* Get operator and arguments */
    f = FUNC(x);
    a = ARGS(x);
    n = listlen(a);

    /* Special form or special case ? */
    if (SYMBP(f)) {

        /* Special forms */

        /* call/cc */
        if (EQNAM(f, CALLCC)) {
            if (!(n == 1)) {
                sprintf(EBUF, "? Arguments to \"%s\": one", CALLCC);
                evalerr(EBUF);
            }
            PUSH(r, C_APPLY);
            r = eload(r, ARG1(a));
            PUSH(r, C_MAKECC);
            return r;
        }

        /* define */
        if (EQNAM(f, DEFINE)) {
            if (!SYMBP(ARG1(a)) || !((n == 1) || (n == 2))) {
                sprintf(EBUF,
                    "? Arguments to \"%s\": one or two, first a symbol",
                        DEFINE);
                evalerr(EBUF);
            }

            /* One arg: after bind, return symbol */
            if (n == 1) {
                PUSH(r, ARG1(a));
                PUSH(r, C_QUOTE);
            } else {

                /* Two args: after bind, assign, return value */
                PUSH(r, ARG1(a));
                PUSH(r, C_DEFINE);
                r = cload(r, ARG2(a));
                r = evaljmp(r);
            }

            /* Bind at this level if necessary */
            PUSH(r, ARG1(a));
            PUSH(r, C_DEFINE);
            return r;
        }

        /* if */
        if (EQNAM(f, IF)) {
            if (!((n == 2) || (n == 3))) {
                sprintf(EBUF, "? Arguments to \"%s\": two or three", IF);
                evalerr(EBUF);
            }
            if (n == 2) {
                r = cload(r, nil);
            } else {
                r = cload(r, ARG3(a));
            }
            r = cload(r, ARG2(a));
            PUSH(r, C_SELECT);
            r = eload(r, ARG1(a));
            return r;
        }

        /* lambda or macro */
        if (EQNAM(f, LAMBDA) || EQNAM(f, MACRO)) {
            if (!n) {
                sprintf(EBUF, "? \"%s\" with no parameter specification",
                    SYMBN(f));
                evalerr(EBUF);
            }

            /* Create closure or macro */
            a = addprogn(a);
            EMPTY(t);
            t = cload(t, CAR(CDR(a)));
            PUSH(t, CAR(a));
            PUSH(r, t);
            PUSH(r, EQNAM(f, LAMBDA) ? C_LAMBDA : C_MACRO);
            return r;
        }

        /* progn */
        if (EQNAM(f, PROGN)) {
            EMPTY(t);
            if (!n) {
                PUSH(r, nil);
                PUSH(r, C_QUOTE);
                return r;
            }
            if (n == 1) {
                r = eload(r, CAR(a));
                return r;
            }
            revlist(a, &t);
            while (!NILP(t)) {
                r = cload(r, CAR(t));
                r = evaljmp(r);
                DROP(t);
            }
            return r;
        }

        /* quote */
        if (EQNAM(f, QUOTE)) {
            if (!(n == 1)) {
                sprintf(EBUF, "? Arguments to \"%s\": one", QUOTE);
                evalerr(EBUF);
            }
            PUSH(r, ARG1(a));
            PUSH(r, C_QUOTE);
            return r;
        }

        /* setq */
        if (EQNAM(f, SETQ)) {
            if (!SYMBP(ARG1(a)) || !(n == 2)) {
                sprintf(EBUF,
                        "? Arguments to \"%s\": two, first a symbol", SETQ);
                evalerr(EBUF);
            }

            /* Search lexical environment */
            if (!(b = esearch(ARG1(a), &idx, &lev))) {

                /* Not found */
                sprintf(EBUF, "? Unbound variable: \"%s\"", SYMBN(ARG1(a)));
                evalerr(EBUF);
            }

            /* Primitive names cannot be reassigned at top level */
            if (ISHARDP(idx, lev)) {
                sprintf(EBUF,
                    "? Top level binding of \"%s\" cannot be changed",
                        SYMBN(ARG1(a)));
                evalerr(EBUF);
            }

            /* Set value in environment */
            if (istop(lev)) {
                PUSH(r, mkaddr(b));
                PUSH(r, C_SETB);
            } else {
                PUSH(r, mkintn((ptrdiff_t)lev));
                PUSH(r, mkintn((ptrdiff_t)idx));
                PUSH(r, C_SETL);
            }
            r = eload(r, ARG2(a));
            return r;
        }

        /* Special cases */
        if ((b = esearch(f, &idx, &lev))) {
            b = CDR(b);

            /* Macro application ? */
            if (ISMACRO(b)) {
                r = mload(r, b, a);
                return r;
            }

            /* Applying a primitive ? */
            if ((pf = ISHARDP(idx, lev))) {
                d = pf->narg;
                if (!(d < 0) && !(d == n)) {
                    sprintf(EBUF, "? Arguments to \"%s\": %d", SYMBN(f), d);
                    evalerr(EBUF);
                }
                PUSH(r, PFCMDF(b));
                f = x;
                DROP(f);
                while (!NILP(f)) {
                    r = eload(r, CAR(f));
                    DROP(f);
                }
                return r;
            }
        }
    }

    /* Anything else is a function application */
    PUSH(r, C_APPLY);
    f = x;
    while (!NILP(f)) {
        r = eload(r, CAR(f));
        DROP(f);
    }

    return r;
}
static DCELL *cexpand(DCELL *x)
{
    DCELL *c, *r;

    EMPTY(r);
    c = exprcmnd(r, x);

    return c;
}

/* Core VM commands */

/* Top 9 most frequently called */
static void c_apply()
{
    DCELL *d, *p, *r;

    /* Applicable ? */
    POP(S, C);
    if (ATOMP(C)) {
        evalerr("? Unknown function type");
    }

    /* Primitive ? */
    if (CMNDP(C)) {
        return;
    }

    /* Lambda-like. Recover environment, bind arguments */
    POP(C, r);
    if (!NILP(r)) {
        E = ADDRF(r);
        EMPTY(r);
    }
    POP(C, p);
    while (PAIRD(p) && !NILP(S)) {

        /* This surgery reuses S cells to make association pairs */
        d = S;
        DROP(S);
        CDR(d) = TOP(d);
        CAR(d) = CAR(p);
        r = mkpair(d, r);
        DROP(p);
    }

    /* More on parameter list ? */
    if (!NILP(p)) {
        if (!SYMBP(p)) {
            evalerr("? Too few values");
        }
        d = mkpair(p, S);
        r = mkpair(d, r);
        EMPTY(S);
    }

    /* More values than things to bind ? */
    if (!NILP(S)) {
        evalerr("? Too many values");
    }

    /* New environment */
    E = mkpair(r, E);

    /* Evaluate the body */
    C = CAR(C);
    evalgc();
}
static void c_get20()
{
    DCELL *v;

    v = CAR(E);
    v = CDR(v);
    v = CDR(v);
    v = CDR(CAR(v));
    PUSH(S, v);
}
static void c_get10()
{
    DCELL *v;

    v = CAR(E);
    v = CDR(v);
    v = CDR(CAR(v));
    PUSH(S, v);
}
static void c_get00()
{
    DCELL *v;

    v = CAR(E);
    v = CDR(CAR(v));
    PUSH(S, v);
}
static void c_getb()
{
    DCELL *a, *v;

    POP(C, a);
    v = CDR(ADDRF(a));
    PUSH(S, v);
}
static void c_evalc()
{
    DCELL *c;

    POP(C, c);
    PUSH(D, mkcont(S, E, C));
    C = c;
    EMPTY(S);

    evalgc();
}
static void c_select()
{
    DCELL *v, *t, *f;

    POP(S, v);
    POP(C, t);
    POP(C, f);
    C = NILP(v) ? f : t;

    evalgc();
}
static void c_quote()
{
    DCELL *x;

    POP(C, x);
    PUSH(S, x);
}
static void c_end()
{
    DCELL *k, *s;

    if (!NILP(D)) {
 
        /* The surgery here puts TOP(S) on top of restored stack */
        POP(D, k);
        s = CONTF(k).s;
        E = CONTF(k).e;
        C = CONTF(k).c;
        CDR(S) = s;
        evalgc();
        return;
    }

    EMPTY(C);
    evalgc();
    LONGJMP(cloopenv, 1);
}

/* Less frequently called */
static void c_argc()
{
    int i, n;
    DCELL *a;
    PFUNC *pf;

    /* Expected number of arguments */
    POP(C, a);
    n = INTNF(a);

    /* Actual number of arguments */
    i = listlen(S);

    /* Error if they don't match */
    if (!(i == n)) {
        pf = ispcmd(TOP(C));
        sprintf(EBUF, "? Arguments to \"%s\": %d", pf->pname, n);
        evalerr(EBUF);
    }
}
static void c_continue()
{
    DCELL *a;

    if (NILP(S) || !NILP(CDR(S))) {
        evalerr("? Arguments to continuation: one");
    }

    POP(C, a);
    D = ADDRF(a);
}
static void c_define()
{
    DCELL *a, *b, *x;

    /* Primitives are materialized, cannot be defined at top level */
    POP(C, x);
    if ((E == topenv) && ispnam(x)) {
        sprintf(EBUF, "? Cannot bind \"%s\" at top level", SYMBN(x));
        evalerr(EBUF);
    }

    /* Search level 0 a-list */
    b = nil;
    while (1) {
        a = CAR(E);
        while (!NILP(a)) {
        
            /* Found ? */
            b = CAR(a);
            if (CAR(b) == x) {
                break;
            }

            DROP(a);
        }

        /* If variable not bound at this level, extend environment */
        if (NILP(a)) {
            eappend(E, x, unval);
            continue;
        }

        break;
    }

    /* Bound, assign value if provided */
    if (!NILP(S)) {
        TOPCHG(b);
        CDR(b) = TOP(S);
    }
}
static void c_getl()
{
    ptrdiff_t i, j, l;
    DCELL *b, *v, *x;

    /* Get index */
    POP(C, x);
    i = INTNF(x);

    /* Get level */
    POP(C, x);
    l = INTNF(x);

    /* Step to level */
    b = E;
    j = 0;
    while (j < l) {
        b = CDR(b);
        ++j;
    }
    b = CAR(b);

    /* Step to right binding */
    j = 0;
    while (j < i) {
        b = CDR(b);
        ++j;
    }
    b = CAR(b);

    /* Get value */
    v = CDR(b);
    PUSH(S, v);
}
static void c_jumpc()
{
    C = CAR(C);
    EMPTY(S);

    evalgc();
}
static void c_jumpm()
{
    C = CAR(C);
}
static void c_lambda()
{
    DCELL *v;

    POP(C, v);
    PUSH(v, mkaddr(E));
    PUSH(S, v);
}
static void c_load()
{
    size_t idx, lev;
    DCELL *a, *b, *p, *t, *x;

    evalgc();

    POP(C, p);
    POP(S, x);

    C = cexpand(x);
    a = ADDRF(p);

    /* Inlining in front of other commands ? */
    if (!NILP(CDR(CDR(a)))) {

        /* These commands all think they are last */
        t = C;
        while (!NILP(t)) {
            if (CAR(t) == C_APPLY) {
                break;
            }
            if (CAR(t) == C_SELECT) {
                break;
            }
            if (CAR(t) == C_JUMPC) {
                break;
            }
            DROP(t);
        }

        /* Have to put this expansion in a sub-expression */
        if (!NILP(t)) {
            EMPTY(t);
            PUSH(t, C);
            PUSH(t, C_EVALC);
            C = t;
        }
    }

    /* Splice in the command list */
    t = C;
    while (!NILP(CDR(t))) {
        DROP(t);
    }
    TOPCHG(t);
    CDR(t) = CDR(CDR(a));

    /* Patching turned off ? */
    if ((b = esearch(MKSYMB(DO_NOT_PATCH), &idx, &lev))) {
        if (!NILP(CDR(b))) {
            return;
        }
    }

    /* Patch VM expansion of macro expansion into call site */
    TOPCHG(a);
    CAR(a) = CAR(C);
    CDR(a) = CDR(C);

    /* Fix any macro LOAD just inside, to patch new position */
    if (CAR(a) == C_JUMPM) {
        t = CAR(CDR(a));
        while (!NILP(t)) {
            if (CAR(t) == C_LOAD) {
                TOPCHG(CDR(t));
                CAR(CDR(t)) = p;
                break;
            }
            DROP(t);
        }
    }
}
static void c_loadc()
{
    size_t idx, lev;
    DCELL *a, *b, *x;

    evalgc();

    POP(C, a);
    POP(C, x);

    C = cexpand(x);
    a = ADDRF(a);

    /* Patching turned off ? */
    if ((b = esearch(MKSYMB(DO_NOT_PATCH), &idx, &lev))) {
        if (!NILP(CDR(b))) {
            return;
        }
    }

    /* Patch VM expansion over load command */
    TOPCHG(a);
    CAR(a) = C;
}
static void c_macro()
{
    DCELL *v;

    POP(C, v);
    PUSH(v, nil);
    PUSH(S, v);
}
static void c_makecc()
{
    DCELL *v;

    EMPTY(v);
    PUSH(v, mkaddr(D));
    PUSH(v, C_CONTINUE);
    PUSH(S, v);
}
static void c_setb()
{
    DCELL *a, *b, *v;

    POP(C, a);
    b = ADDRF(a);
    v = TOP(S);
    TOPCHG(b);
    CDR(b) = v;
}
static void c_setl()
{
    ptrdiff_t i, j, l;
    DCELL *b, *v, *x;

    /* Get index */
    POP(C, x);
    i = INTNF(x);

    /* Get level */
    POP(C, x);
    l = INTNF(x);

    /* Step to level */
    b = E;
    j = 0;
    while (j < l) {
        b = CDR(b);
        ++j;
    }
    b = CAR(b);

    /* Step to right binding */
    j = 0;
    while (j < i) {
        b = CDR(b);
        ++j;
    }
    b = CAR(b);

    /* Set value */
    v = TOP(S);
    TOPCHG(b);
    CDR(b) = v;
}

/* eval is command loop */
static void eval()
{
    size_t idx, lev;
    DCELL *h, *x;
    void (*oldebrkh)(), (*oldefpeh)();

    oldebrkh = hookbrk(ebrkhand);
    oldefpeh = hookfpe(efpehand);

    /* Loop on commands until we run out or error */
    if (!SETJMP(evalenv)) {
        POP(S, x);
        C = cexpand(x);
        while (1) {
            if (!SETJMP(evalenv)) {
                if (!SETJMP(cloopenv)) {
                    while (1) {
                        CALLC();
                    }
                }

                hookbrk(oldebrkh);
                hookfpe(oldefpeh);
                return;
            }

            /* Run-time error comes here - is there a handler set ? */
            if ((h = esearch(MKSYMB(ERROR_CONTINUE), &idx, &lev))) {
                C = CDR(h);

                /* Must be a reified continuation */
                if (CAR(C) == C_CONTINUE) {
                    EMPTY(S);
                    PUSH(S, MKSYMB(EBUF));
                    continue;
                }
            }

            break;
        }
    }

    /* Reset top state and push error symbol */
    EMPTY(S);
    E = topenv;
    EMPTY(C);
    EMPTY(D);
    PUSH(S, MKSYMB(EBUF));

    hookbrk(oldebrkh);
    hookfpe(oldefpeh);
}

/* Print VM commands */ 
static void vcmdprint(c)
DCELL *c;
{
    /* Command ? */
    if (c == C_APPLY) {
        OPUTS("APPLY");
        return;
    }
    if (c == C_ARGC) {
        OPUTS("ARGC");
        return;
    }
    if (c == C_CONTINUE) {
        OPUTS("CONTINUE");
        return;
    }
    if (c == C_DEFINE) {
        OPUTS("DEFINE");
        return;
    }
    if (c == C_EVALC) {
        OPUTS("EVALC");
        return;
    }
    if (c == C_GETB) {
        OPUTS("GETB");
        return;
    }
    if (c == C_GETL) {
        OPUTS("GETL");
        return;
    }
    if (c == C_GET00) {
        OPUTS("GET00");
        return;
    }
    if (c == C_GET10) {
        OPUTS("GET10");
        return;
    }
    if (c == C_GET20) {
        OPUTS("GET20");
        return;
    }
    if (c == C_JUMPC) {
        OPUTS("JUMPC");
        return;
    }
    if (c == C_JUMPM) {
        OPUTS("JUMPM");
        return;
    }
    if (c == C_LAMBDA) {
        OPUTS("LAMBDA");
        return;
    }
    if (c == C_LOAD) {
        OPUTS("LOAD");
        return;
    }
    if (c == C_LOADC) {
        OPUTS("LOADC");
        return;
    }
    if (c == C_MACRO) {
        OPUTS("MACRO");
        return;
    }
    if (c == C_MAKECC) {
        OPUTS("MAKECC");
        return;
    }
    if (c == C_QUOTE) {
        OPUTS("QUOTE");
        return;
    }
    if (c == C_SELECT) {
        OPUTS("SELECT");
        return;
    }
    if (c == C_SETB) {
        OPUTS("SETB");
        return;
    }
    if (c == C_SETL) {
        OPUTS("SETL");
        return;
    }

    /* Unknown commands */
    OPUTS("???");
}

/* Initialize evaluator - set up the starting environment */
static void init_eval()
{
    /* Environment starts empty */
    PUSH(E, nil);
    topenv = E;

    /* Truth, unassigned value */
    tval = MKSYMB(TVAL);
    unval = mknil();

    /* Gensym counter */
    gsymc = 0;

    /* Forget enabled or not */
    forget = FORGET;

    /* Break handler state */
    dontbrk = ebrk = 0;
    ebrkstr = istty(stdin) ? "^C" : "\n";
}

/******************************************************************************/
/* Foreign functions                                                          */
/******************************************************************************/

/* Function links */
typedef void *(*PFVP)(void *a1, void *a2, void *a3);
typedef struct ffun {
    char *ffname;
    PFVP ffptr;
} FFUN;

/* Arguments typed and staged */
static int dtyp[3];
static double darg[3];
static void *args[3];

/* C types supported */
static char *ctypes[] = {
    "char*",
    "int",
    "double",
    "void*",
    "?",
    NULL
};

/* Standard I/O streams */
static void *_stdin()
{
    return stdin;
}
static void *_stdout()
{
    return stdout;
}
static void *_stderr()
{
    return stderr;
}

/* Adaptors for math functions */
static void *myceil(void *pn)
{
    *(double *)pn = ceil(*(double *)pn);

    return pn;
}
static void *myfloor(void *pn)
{
    *(double *)pn = floor(*(double *)pn);

    return pn;
}
static void *mypow(void *pn1, void *pn2)
{
    *(double *)pn1 = pow(*(double *)pn1, *(double *)pn2);

    return pn1;
}
static void *mysqrt(void *pn)
{
    *(double *)pn = sqrt(*(double *)pn);

    return pn;
}

/* Adaptor for sprintf */
static void *mysprintf(char *fmt, void *dat)
{
    char *buf;

    buf = TBUF1;
    if (!strcmp(ctypes[dtyp[1]], "double")) {
        sprintf(buf, fmt, *(double *)dat);
    } else {
        sprintf(buf, fmt, dat);
    }

    return buf;
}

/* Foreign functions */
static FFUN fftab[] = {

    /* ctype.h */
    { "isalpha", (PFVP)isalpha },
    { "isdigit", (PFVP)isdigit },
    { "isspace", (PFVP)isspace },
    { "toupper", (PFVP)toupper },

    /* math.h */
    { "ceil", (PFVP)myceil },
    { "floor", (PFVP)myfloor },
    { "pow", (PFVP)mypow },
    { "sqrt", (PFVP)mysqrt },

    /* stdio.h */
    { "stdin", (PFVP)_stdin },
    { "stdout", (PFVP)_stdout },
    { "stderr", (PFVP)_stderr },
    { "fopen", (PFVP)fopen },
    { "fclose", (PFVP)fclose },
    { "fgetc", (PFVP)fgetc },
    { "fputc", (PFVP)fputc },
    { "rewind", (PFVP)rewind },
    { "sprintf", (PFVP)mysprintf },
    { "ungetc", (PFVP)ungetc },

    /* stdlib.h */
    { "atoi", (PFVP)atoi },
    { "getenv", (PFVP)getenv },

    { NULL, NULL }
};

/* Foreign function call primitive */
static void c_ffcall()
{
    char *c;
    int i, j, k, l;
    void *r;
    DCELL *a, *f, *s, *v;
    FFUN *t;
    void (*oldbrk)();

    /* Expect function name, signature, list of arguments */
    POP(S, f);
    POP(S, s);
    v = TOP(S);
    if (!SYMBP(f) || !PAIRP(s) || !(PAIRP(v) || NILP(v))) {
        strcpy(EBUF,
            "? 'ffcall' expects function name, signature, and argument list");
        evalerr(EBUF);
    }

    /* Do we have this function ? */
    c = (char *)SYMBN(f);
    i = 0;
    while (1) {
        t = &fftab[i];
        if (!t->ffname) {
            break;
        }
        if (!strcmp(c, t->ffname)) {
            break;
        }
        ++i;
    }
    if (!t->ffname) {
        sprintf(EBUF, "? Unknown foreign function \"%s\"", c);
        evalerr(EBUF);
    }

    /* Recognized result type in signature ? */
    strcpy(EBUF, "? Improper signature for foreign function");
    a = CAR(s);
    k = 0;
    while (SYMBP(a) && ctypes[k]) {
        if (!strcmp(ctypes[k], SYMBN(a))) {
            break;
        }
        ++k;
    }
    if (!SYMBP(a) || !ctypes[k] || !strcmp(ctypes[k], "?")) {
        evalerr(EBUF);
    }

    /* Count, check, convert, marshall arguments */
    a = CDR(s);
    if (NILP(a) || !(PAIRP(CAR(a)) || NILP(CAR(a)))) {
        evalerr(EBUF);
    }
    a = CAR(a);
    v = TOP(S);
    j = 0;
    while ((j < 3) && !NILP(a) && !NILP(v)) {

        /* Recognized type in signature ? */
        i = 0;
        while (SYMBP(CAR(a)) && ctypes[i]) {
            if (!strcmp(ctypes[i], SYMBN(CAR(a)))) {
                break;
            }
            ++i;
        }
        if (!SYMBP(CAR(a)) || !ctypes[i]) {
            evalerr(EBUF);
        }

        /* Does/can, argument lisp type match C type ? */
        dtyp[j] = i;
        if (!strcmp(ctypes[i], "char*")) {
            if (!SYMBP(CAR(v))) {
                break;
            }
            args[j] = (void *)SYMBN(CAR(v));
        }
        if (!strcmp(ctypes[i], "int")) {
            if (!INTNP(CAR(v))) {
                break;
            }
            args[j] = (void *)(ptrdiff_t)(int)INTNF(CAR(v));
        }
        if (!strcmp(ctypes[i], "double")) {
            if (!NUMBP(CAR(v))) {
                break;
            }
            darg[j] = (double)NUMBF(CAR(v));
            args[j] = (void *)&darg[j];
        }
        if (!strcmp(ctypes[i], "void*")) {
            if (!INTNP(CAR(v))) {
                break;
            }
            args[j] = (void *)INTNF(CAR(v));
        }

        /* "?" matches lisp equivalents of all supported C types */
        if (!strcmp(ctypes[i], "?")) {
            l = 0;

            /* char* */
            if (SYMBP(CAR(v))) {
                args[j] = (void *)SYMBN(CAR(v));
                l = 1;
                dtyp[j] = 0;
            }

            /* void* */
            if (INTNP(CAR(v))) {
                args[j] = (void *)INTNF(CAR(v));
                l = 1;
                dtyp[j] = 3;
            }

            /* double */
            if (NUMBP(CAR(v))) {
                darg[j] = (double)NUMBF(CAR(v));
                args[j] = (void *)&darg[j];
                l = 1;
                dtyp[j] = 2;
            }
            if (!l) {
                break;
            }
        }

        DROP(a);
        DROP(v);
        ++j;
    }
    if (!NILP(a) || !NILP(v)) {
        evalerr("? Foreign function arguments don't match signature");
    }

    /* Set immediate handler, take deferred break */
    oldbrk = NULL;
    if (!dontbrk) {
        oldbrk = hookbrk(ebrkjmp);
        if (ebrk) {
            ebrkjmp();
        }
    }

    /* Call C function */
    r = (t->ffptr)(args[0], args[1], args[2]);

    /* Restore deferred handler */
    if (!dontbrk) {
        hookbrk(oldbrk);
    }

    /* Convert C result back to appropriate lisp type */
    if (!strcmp(ctypes[k], "char*")) {
        if (!r) {
            r = "";
        }
        v = MKSYMB((char *)r);
    }
    if (!strcmp(ctypes[k], "int")) {
        v = mkintn((ptrdiff_t)(int)(ptrdiff_t)r);
    }
    if (!strcmp(ctypes[k], "double")) {
        v = mknumb(*(double *)r);
    }
    if (!strcmp(ctypes[k], "void*")) {
        v = mkintn((ptrdiff_t)r);
    }

    TOP(S) = v;
}

/******************************************************************************/
/* Primitives                                                                 */
/******************************************************************************/

/* Signature checks */
#define INT2P(x, y) (INTNP(x) && INTNP(y))
#define NUM2P(x, y) (NUMBP(x) && NUMBP(y))

#define ARGS1LIST() \
    (listp(ARG1(S)))
#define ARGS1INTN() \
    (INTNP(ARG1(S)))
#define ARGS1NUMB() \
    (INTNP(ARG1(S)) || NUMBP(ARG1(S)))
#define ARGS1PAIR() \
    (PAIRP(ARG1(S)) && !CMNDV(ARG1(S)))
#define ARGS1SYMB() \
    (SYMBP(ARG1(S)))
#define ARGS1VECT() \
    (VECTP(ARG1(S)))
#define ARGS2INT() \
    (INT2P(ARG1(S), ARG2(S)))
#define ARGS2SYMB() \
    (SYMBP(ARG1(S)) && SYMBP(ARG2(S)))
#define VREFARGS() \
    (VECTP(ARG1(S)) && INTNP(ARG2(S)))

/* One argument, an integer */
static void oneintn(char *s)
{
    if (!ARGS1INTN()) {
        sprintf(EBUF, "? Argument to \"%s\" must be an integer", s);
        evalerr(EBUF);
    }
}

/* One argument, a list */
static void onelist(char *s)
{
    if (!ARGS1LIST()) {
        sprintf(EBUF, "? Argument to \"%s\" must be a list", s);
        evalerr(EBUF);
    }
}

/* One argument, a number */
static void onenumb(char *s)
{
    if (!ARGS1NUMB()) {
        sprintf(EBUF, "? Argument to \"%s\" must be a number", s);
        evalerr(EBUF);
    }
}

/* One argument, a pair */
static void onepair(char *s)
{
    if (!ARGS1PAIR()) {
        sprintf(EBUF, "? \"%s\" must be a pair", s);
        evalerr(EBUF);
    }
}

/* Mutable pair */
static void mpair(char *s)
{
    if (!PAIRD(ARG1(S))) {
        sprintf(EBUF, "? First argument to \"%s\" must be a mutable pair", s);
        evalerr(EBUF);
    }
}

/* One argument, a symbol */
static void onesymb(char *s)
{
    if (!ARGS1SYMB()) {
        sprintf(EBUF, "? Argument to \"%s\" must be a symbol", s);
        evalerr(EBUF);
    }
}

/* One argument, a vector */
static void onevect(char *s)
{
    if (!ARGS1VECT()) {
        sprintf(EBUF,
                "? Argument to \"%s\" must be a vector", s);
        evalerr(EBUF);
    }
}

/* Two arguments, both integers */
static void twoint(char *s)
{
    if (!ARGS2INT()) {
        sprintf(EBUF, "? Arguments to \"%s\" must be two integers", s);
        evalerr(EBUF);
    }
}

/* Two arguments, both real numbers */
static void twonumb(char *s)
{
    sprintf(EBUF, "? Arguments to \"%s\" must be two numbers", s);
    evalerr(EBUF);
}

/* Two arguments, both symbols */
static void twosymb(char *s)
{
    if (!ARGS2SYMB()) {
        sprintf(EBUF, "? Arguments to \"%s\" must be two symbols", s);
        evalerr(EBUF);
    }
}

/* vref, vset: first two args are vector, integer */
static void vrefargs(char *s)
{
    if (!VREFARGS()) {
        sprintf(EBUF, "? %s must be vector, integer", s);
        evalerr(EBUF);
    }   
}

/* Primitive function implementations - eval related */
static void c_assignedp()
{
    size_t idx, lev;
    DCELL *b, *v, *x;

    onesymb("assignedp");

    POP(S, x);
    v = nil;
    if ((b = esearch(x, &idx, &lev))) {
        if (!(CDR(b) == unval)) {
            v = tval;
        }
    }
    PUSH(S, v);
}
static void c_boundp()
{
    size_t idx, lev;
    DCELL *v, *x;

    onesymb("boundp");

    POP(S, x);
    v = esearch(x, &idx, &lev) ? tval : nil;
    PUSH(S, v);
}
static void c_eval()
{
    int n;
    DCELL *d, *k, *x;

    evalgc();

    /* One or two arguments, 2nd must be continuation */
    n = listlen(S);
    if ((n < 1) || (2 < n) || \
        ((n == 2) && !(CAR(ARG2(S)) == C_CONTINUE))) {
        evalerr("? Arguments to \"eval\": one or two, second a continuation");
    }

    /* Expression to evaluate */
    POP(S, x);

    /* Optional argument is continuation holding eval environment */
    if (!NILP(S)) {
        POP(S, k);
        d = ADDRF(CAR(CDR(k)));
        E = CONTF(CAR(d)).e;
    }
        
    C = cexpand(x);
}
static void c_forget()
{
    DCELL *a0, *v;

    onesymb("forget");

    a0 = TOP(S);
    v = eforget(a0) ? tval : nil;
    TOP(S) = v;
}
static void c_setetop()
{
    DCELL *a0, *a1, *v;

    onesymb("setetop");

    POP(S, a0);
    a1 = TOP(S);
    v = setetop(a0, a1);
    TOP(S) = v;
}

/* Debugger primitive function */
static void c_debugger()
{
    int bb, i, ml;
    PFUNC *pf;
    DCELL *c, *dbcode, *f;

    /* If returning from repl, drop return value */
    if (!NILP(S)) {
        DROP(S);
    }

    /* Display function name */
    POP(C, f);
    exprint(f);
    putchar('\n');

    /* Get command list to execute */
    POP(C, c);
    C = c;

    /* Display state, get debugger commands and continue execution */
    pf = ispnam(MKSYMB(DEBUGGER));
    dbcode = (DCELL *)pf->fcode;
    bb = i = ml = 0;
    while (!NILP(D)) {

        evalgc();

        /* Entering a macro expansion ? */
        if (CAR(C) == C_JUMPM) {
            ++ml;
        }

        /* Stop at this state ? */
        if (!ml && (!(i == (int)'n') || bb)) {

            /* Show S, E(0), C, D */
            fputs("S: ", stdout);
            exprint(S);
            putchar('\n');
            fputs("E: ", stdout);
            exprint(CAR(E));
            putchar('\n');
            fputs("C: ", stdout);
            exprint(C);
            putchar('\n');
            fputs("D: ", stdout);
            exprint(D);
            putchar('\n');

            /* Get a debug command */
            fputs(">> ", stdout);
            i = getchar();
            if (!(i == EOT)) {
                putchar(i);
            }
            if (!(i == NL)) {
                putchar('\n');
            }
        }

        /* Completing a macro expansion ? */
        if (CAR(C) == C_LOAD) {
            --ml;
        }

        /* Exit ? */
        if ((i == EOT) || (i == 'x')) {
            evalerr("");
        }

        /* Drop into repl ? */
        if (i == 'r') {
            c = C;
            EMPTY(C);
            PUSH(C, c);
            PUSH(C, MKSYMB(""));
            PUSH(C, dbcode);
            PUSH(D, mkcont(S, E, C));
            EMPTY(f);
            PUSH(f, MKSYMB(DREPL));
            C = cexpand(f);
            EMPTY(S);
            break;
        }

        /* Step */
        if ((i == 'n') || (i == 's')) {

            /* Stepping to next 'basic block' ? */
            bb = ((CAR(C) == C_APPLY) || (CAR(C) == C_END) || \
                  (CAR(C) == C_EVALC) || (CAR(C) == C_JUMPC) || \
                  (CAR(C) == C_SELECT) || (CAR(C) == dbcode));

            /* Execute one VM instruction */
            CALLC();
            continue;
        }
        
        break;
    }
}

/* Primitive function implementations */
static void c_add()
{
    DCELL *a0, *a1, *v;

    POP(S, a0);
    a1 = TOP(S);

    if (INT2P(a0, a1)) {
        v = mkintn(INTNF(a0) + INTNF(a1));
        TOP(S) = v;
        return;
    }
    if (INTNP(a0)) {
        a0 = mknumb((double)INTNF(a0));
    }
    if (INTNP(a1)) {
        a1 = mknumb((double)INTNF(a1));
    }
    if (NUM2P(a0, a1)) {
        v = mknumb(NUMBF(a0) + NUMBF(a1));
        TOP(S) = v;
        return;
    }

    twonumb("_add");
}
static void c_asr()
{
    DCELL *a0, *a1, *v;

    twoint(">>");
    POP(S, a0);
    a1 = TOP(S);
    v = mkintn(INTNF(a0) >> INTNF(a1));
    TOP(S) = v;
}
static void c_car()
{
    char c[2], *s;
    DCELL *a0, *v;

    a0 = TOP(S);
    if (NILP(a0)) {
        return;
    }

    if (SYMBP(a0)) {
        s = SYMBN(a0);
        if (SYMBL(a0)) {
            c[0] = *s;
            c[1] = '\0';
            v = MKSYMB(c);
            TOP(S) = v;
        }
        return;
    }

    onepair("Argument to 'car', if not () or symbol,");
    v = CAR(a0);
    TOP(S) = v;
}
static void c_cats()
{
    char *buf;
    int m, n;
    DCELL *a0, *a1, *v;

    twosymb("_cats");

    evalgc();

    POP(S, a0);
    a1 = TOP(S);
    m = SYMBL(a0);
    n = SYMBL(a1);
    if (MAXSYMLEN < (m + n)) {
        sprintf(EBUF, "? _cats: length (%d) exceeds maximum (%d) for symbols",
                m + n, MAXSYMLEN);
        evalerr(EBUF);
    }
    buf = TBUF1;
    memcpy(buf, SYMBN(a0), m);
    memcpy(&buf[m], SYMBN(a1), n);
    buf[m + n] = '\0';
    v = mksymb(buf, m + n);
    TOP(S) = v;

    evalgc();
}
static void c_cdr()
{
    char *s;
    DCELL *a0, *v;

    a0 = TOP(S);
    if (NILP(a0)) {
        return;
    }

    if (SYMBP(a0)) {
        s = SYMBN(a0);
        if (SYMBL(a0)) {
            v = MKSYMB(&s[1]);
            TOP(S) = v;
        }
        return;
    }

    onepair("Argument to 'cdr', if not () or symbol,");
    v = CDR(a0);
    TOP(S) = v;
}
static void c_cons()
{
    DCELL *a0, *a1, *v;

    POP(S, a0);
    a1 = TOP(S);
    v = CONS(a0, a1);
    TOP(S) = v;
}
static void c_defer()
{
    DCELL *a0, *v;

    a0 = TOP(S);
    v = dontbrk ? tval : nil;
    dontbrk = NILP(a0) ? 0 : 1;
    TOP(S) = v;
}
static void c_div()
{
    DCELL *a0, *a1, *v;
    ptrdiff_t id;

    POP(S, a0);
    a1 = TOP(S);

    if (INT2P(a0, a1)) {
        id = INTNF(a1);
        if (!id) {
            evalerr("? Divide by zero");
        }

        v = mkintn(INTNF(a0) / id);
        TOP(S) = v;
        return;
    }
    if (INTNP(a0)) {
        a0 = mknumb((double)INTNF(a0));
    }
    if (INTNP(a1)) {
        a1 = mknumb((double)INTNF(a1));
    }
    if (NUM2P(a0, a1)) {
        v = mknumb(NUMBF(a0) / NUMBF(a1));
        TOP(S) = v;
        return;
    }

    twonumb("_div");
}
static void c_eof()
{
    DCELL *v;

    eof();
    v = MKSYMB("");
    PUSH(S, v);
}
static void c_eq()
{
    DCELL *a0, *a1, *v;

    POP(S, a0);
    a1 = TOP(S);
    v = (a0 == a1) ? tval : nil;
    TOP(S) = v;
}
static void c_equal()
{
    DCELL *a0, *a1, *v;

    POP(S, a0);
    a1 = TOP(S);

    if (INT2P(a0, a1)) {
        v = (INTNF(a0) == INTNF(a1)) ?  tval : nil;
        TOP(S) = v;
        return;
    }
    if (INTNP(a0)) {
        a0 = mknumb((double)INTNF(a0));
    }
    if (INTNP(a1)) {
        a1 = mknumb((double)INTNF(a1));
    }
    if (NUM2P(a0, a1)) {
        v = (NUMBF(a0) == NUMBF(a1)) ?  tval : nil;
        TOP(S) = v;
        return;
    }

    twonumb("_equal");
}
static void c_error()
{
    DCELL *a0;

    onesymb("error");
    POP(S, a0);
    evalerr(SYMBN(a0));
}
static void c_gensym()
{
    char *buf;
    DCELL *v;

    buf = TBUF1;
    sprintf(buf, "g;%ld", gsymc);
    v = MKSYMB(buf);
    ++gsymc;
    PUSH(S, v);
}
static void c_iand()
{
    DCELL *a0, *a1, *v;

    twoint("_iand");
    POP(S, a0);
    a1 = TOP(S);
    v = mkintn(INTNF(a0) & INTNF(a1));
    TOP(S) = v;
}
static void c_imod()
{
    DCELL *a0, *a1, *v;
    ptrdiff_t id;

    twoint("%");
    POP(S, a0);
    a1 = TOP(S);
    id = INTNF(a1);
    if (!id) {
        evalerr("? Divide by zero");
    }
    v = mkintn(INTNF(a0) % id);
    TOP(S) = v;
}
static void c_inclevel()
{
    DCELL *v;

    v = mkintn(finsp);
    PUSH(S, v);
}
static void c_include()
{
    int e;
    DCELL *a0, *v;

    onesymb("include");

    a0 = TOP(S);
    if ((e = include(SYMBN(a0)))) {
        switch (e) {
            case 1 :
                strcpy(EBUF, "? Include file stack full");
                break;
            default :
                sprintf(EBUF, "? Can't include file \"%s\"", SYMBN(a0));
                break;
        }
        evalerr(EBUF);
    }

    v = MKSYMB("");
    TOP(S) = v;
}
static void c_inot()
{
    DCELL *a0, *v;

    oneintn("~");
    a0 = TOP(S);
    v = mkintn(~INTNF(a0));
    TOP(S) = v;
}
static void c_int2real()
{
    ptrdiff_t n;
    DCELL *a0, *v;

    oneintn("int2real");
    a0 = TOP(S);
    n = INTNF(a0);
    v = mknumb((double)n);
    TOP(S) = v;
}
static void c_int2sym()
{
    char buf[2];
    DCELL *a0, *v;

    oneintn("int2sym");
    a0 = TOP(S);
    buf[0] = (char)INTNF(a0);
    buf[1] = '\0';
    v = mksymb(buf, 1);
    TOP(S) = v;
}
static void c_integerp()
{
    DCELL *v;

    v = INTNP(TOP(S)) ? tval : nil;
    TOP(S) = v;
}
static void c_ior()
{
    DCELL *a0, *a1, *v;

    twoint("_ior");
    POP(S, a0);
    a1 = TOP(S);
    v = mkintn(INTNF(a0) | INTNF(a1));
    TOP(S) = v;
}
static void c_ixor()
{
    DCELL *a0, *a1, *v;

    twoint("_ixor");
    POP(S, a0);
    a1 = TOP(S);
    v = mkintn(INTNF(a0) ^ INTNF(a1));
    TOP(S) = v;
}
static void c_led()
{
    DCELL *a0;

    a0 = TOP(S);
    led(!NILP(a0));
}
static void c_less()
{
    DCELL *a0, *a1, *v;

    POP(S, a0);
    a1 = TOP(S);

    if (INT2P(a0, a1)) {
        v = (INTNF(a0) < INTNF(a1)) ?  tval : nil;
        TOP(S) = v;
        return;
    }
    if (INTNP(a0)) {
        a0 = mknumb((double)INTNF(a0));
    }
    if (INTNP(a1)) {
        a1 = mknumb((double)INTNF(a1));
    }
    if (NUM2P(a0, a1)) {
        v = (NUMBF(a0) < NUMBF(a1)) ?  tval : nil;
        TOP(S) = v;
        return;
    }

    twonumb("_less");
}
static void c_list()
{
    DCELL *v;

    v = S;
    EMPTY(S);
    PUSH(S, v);
}
static void c_list2vect()
{
    int n;
    DCELL *a0, *v;

    onelist("list2vector");

    evalgc();

    a0 = TOP(S);
    n = listlen(a0);
    if (MAXVECLEN < n) {
        sprintf(EBUF,
                "? list2vector: length (%d) exceeds maximum (%d) for vectors",
                n, MAXVECLEN);
        evalerr(EBUF);
    }
    v = mkvect(a0);
    TOP(S) = v;

    evalgc();
}
static void c_ls()
{
    ls();
    PUSH(S, MKSYMB(""));
}
static void c_mul()
{
    DCELL *a0, *a1, *v;

    POP(S, a0);
    a1 = TOP(S);

    if (INT2P(a0, a1)) {
        v = mkintn(INTNF(a0) * INTNF(a1));
        TOP(S) = v;
        return;
    }
    if (INTNP(a0)) {
        a0 = mknumb((double)INTNF(a0));
    }
    if (INTNP(a1)) {
        a1 = mknumb((double)INTNF(a1));
    }
    if (NUM2P(a0, a1)) {
        v = mknumb(NUMBF(a0) * NUMBF(a1));
        TOP(S) = v;
        return;
    }

    twonumb("_mul");
}
static void c_not()
{
    DCELL *v;

    v = NILP(TOP(S)) ? tval : nil;
    TOP(S) = v;
}
static void c_pairp()
{
    DCELL *v;

    v = ARGS1PAIR() ? tval : nil;
    TOP(S) = v;
}
static void c_print()
{
    DCELL *a0, *v;
    void (*oldbrk)();

    a0 = TOP(S);

    /* Set immediate handler, take deferred break */
    oldbrk = NULL;
    if (!dontbrk) {
        oldbrk = hookbrk(ebrkjmp);
        if (ebrk) {
            ebrkjmp();
        }
    }

    exprint(a0);

    /* Restore deferred handler */
    if (!dontbrk) {
        hookbrk(oldbrk);
    }

    v = MKSYMB("");
    TOP(S) = v;
}
static void c_quit()
{
    done = 1;
    repljmp();
}
static void c_read()
{
    char *ps;
    DCELL *a, *v;
    void (*oldbrk)();

    /* Optional prompt */
    ps = NULL;
    if (!NILP(S) && SYMBP(TOP(S))) {
        POP(S, a);
        ps = SYMBN(a);
    }
    if (!NILP(S)) {
        evalerr("? Arguments to \"read\": zero, or one prompt symbol");
    }

    /* Catch breaks here, take deferred break */
    oldbrk = hookbrk(crbrkhand);
    if (ebrk) {
        ebrkjmp();
    }

    /* Read and handle breaks */
    if (!SETJMP(creadenv)) {
        v = exread(ps);
    } else {
        v = MKSYMB("^C");
        endread(0);
    }

    /* Restore deferred handler, return result */
    hookbrk(oldbrk);
    PUSH(S, v);
}
static void c_readreg()
{
    int d;
    DCELL *a0, *v;

    oneintn("readreg");

    a0 = TOP(S);
    d = readreg(INTNF(a0));
    v = mkintn(d);
    TOP(S) = v;
}
static void c_real2int()
{
    double r;
    DCELL *a0, *v;

    onenumb("real2int");
    a0 = TOP(S);
    r = NUMBF(a0);
    v = mkintn((ptrdiff_t)r);
    TOP(S) = v;
}
static void c_realp()
{
    DCELL *v;

    v = NUMBP(TOP(S)) ? tval : nil;
    TOP(S) = v;
}
static void c_rplaca()
{
    DCELL *a0, *v;

    mpair("rplaca");

    POP(S, a0);
    v = TOP(S);

    TOPCHG(a0);
    CAR(a0) = v;
    TOP(S) = a0;
}
static void c_rplacd()
{
    DCELL *a0, *v;

    mpair("rplacd");

    POP(S, a0);
    v = TOP(S);

    TOPCHG(a0);
    CDR(a0) = v;
    TOP(S) = a0;
}
static void c_shl()
{
    DCELL *a0, *a1, *v;

    twoint("<<");
    POP(S, a0);
    a1 = TOP(S);
    v = mkintn(INTNF(a0) << INTNF(a1));
    TOP(S) = v;
}
static void c_sub()
{
    DCELL *a0, *a1, *v;

    POP(S, a0);
    a1 = TOP(S);

    if (INT2P(a0, a1)) {
        v = mkintn(INTNF(a0) - INTNF(a1));
        TOP(S) = v;
        return;
    }
    if (INTNP(a0)) {
        a0 = mknumb((double)INTNF(a0));
    }
    if (INTNP(a1)) {
        a1 = mknumb((double)INTNF(a1));
    }
    if (NUM2P(a0, a1)) {
        v = mknumb(NUMBF(a0) - NUMBF(a1));
        TOP(S) = v;
        return;
    }

    twonumb("_sub");
}
static void c_sym2int()
{
    ptrdiff_t n;
    DCELL *a0, *v;

    onesymb("sym2int");
    a0 = TOP(S);
    n = (unsigned char)(SYMBN(a0))[0];
    v = mkintn(n);
    TOP(S) = v;
}
static void c_symbolp()
{
    DCELL *v;

    v = SYMBP(TOP(S)) ? tval : nil;
    TOP(S) = v;
}
static void c_vectorp()
{
    DCELL *v;

    v = VECTP(TOP(S)) ? tval : nil;
    TOP(S) = v;
}
static void c_verbose()
{
    DCELL *a0, *v;

    oneintn("verbose");

    v = mkintn((ptrdiff_t)verbose);
    a0 = TOP(S);
    verbose = (int)INTNF(a0);
    TOP(S) = v;
}
static void c_vlen()
{
    DCELL *a0, *v;

    onevect("vlen");
    a0 = TOP(S);
    v = mkintn((ptrdiff_t)VECTL(a0));
    TOP(S) = v;
}
static void c_vref()
{
    ptrdiff_t i;
    DCELL *a0, *a1, *v;

    vrefargs("Arguments to \"vref\"");
    POP(S, a0);
    a1 = TOP(S);
    i = INTNF(a1);
    if ((i < 0) || !((size_t)i < VECTL(a0))) {
        evalerr("? vref: index out of bounds");
    }
    v = VREFF(a0, i);
    TOP(S) = v;
}
static void c_vset()
{
    ptrdiff_t i;
    DCELL *a0, *a1, *v;

    vrefargs("First two arguments to \"vset\"");
    POP(S, a0);
    POP(S, a1);
    v = TOP(S);
    i = INTNF(a1);
    if ((i < 0) || !((size_t)i < VECTL(a0))) {
        evalerr("? vset: index out of bounds");
    }
    TOPCHG(VREFF(a0, i));
    VREFF(a0, i) = v;
    TOP(S) = a0;
}
static void c_writereg()
{
    int d;
    DCELL *a0, *a1;

    twoint("writereg");

    POP(S, a0);
    a1 = TOP(S);
    d = (unsigned char)INTNF(a1);
    writereg(INTNF(a0), d);
}
static void c_zerop()
{
    DCELL *a0, *v;

    a0 = TOP(S);
    if (INTNP(a0)) {
        v = (INTNF(a0) == 0) ? tval : nil;
        TOP(S) = v;
        return;
    }
    if (NUMBP(a0)) {
        v = (NUMBF(a0) == 0.0) ? tval : nil;
        TOP(S) = v;
        return;
    }

    onenumb("zerop");
}

/* Primitive function table */
static PFUNC pftbl[] = {
    { "_add", "ADD", 2, c_add },
    { "_cats", "CATS", 2, c_cats },
    { "_div", "DIV", 2, c_div },
    { "_equal", "EQUAL", 2, c_equal },
    { "_iand", "IAND", 2, c_iand },
    { "_ior", "IOR", 2, c_ior },
    { "_ixor", "IXOR", 2, c_ixor },
    { "_less", "LESS", 2, c_less },
    { "_mul", "MUL", 2, c_mul },
    { "_sub", "SUB", 2, c_sub },
    { "%", "IMOD", 2, c_imod },
    { "assignedp", "ASSIGNEDP", 1, c_assignedp },
    { "boundp", "BOUNDP", 1, c_boundp },
    { "car", "CAR", 1, c_car },
    { "cdr", "CDR", 1, c_cdr },
    { "cons", "CONS", 2, c_cons },
    { "debugger", "DEBUGGER", -1, c_debugger },
    { "defer", "DEFER", 1, c_defer },
    { "eof", "EOF", 0, c_eof },
    { "eq", "EQ", 2, c_eq },
    { "error", "ERROR", 1, c_error },
    { "eval", "EVAL", -1, c_eval },
    { "forget", "FORGET", 1, c_forget },
    { "gensym", "GENSYM", 0, c_gensym },
    { "ffcall", "FFCALL", 3, c_ffcall },
    { "inclevel", "INCLEVEL", 0, c_inclevel },
    { "include", "INCLUDE", 1, c_include },
    { "int2real", "INT2REAL", 1, c_int2real },
    { "int2sym", "INT2SYM", 1, c_int2sym },
    { "integerp", "INTNP", 1, c_integerp },
    { "led", "LED", 1, c_led },
    { "list", "LIST", -1, c_list },
    { "list2vector", "LIST2VECT", 1, c_list2vect },
    { "ls", "LS", 0, c_ls },
    { "not", "NOT", 1, c_not },
    { "pairp", "PAIRP", 1, c_pairp },
    { "print", "PRINT", 1, c_print },
    { "quit", "QUIT", 0, c_quit },
    { "read", "READ", -1, c_read },
    { "readreg", "READREG", 1, c_readreg },
    { "real2int", "REAL2INT", 1, c_real2int },
    { "realp", "REALP", 1, c_realp },
    { "rplaca", "RPLACA", 2, c_rplaca },
    { "rplacd", "RPLACD", 2, c_rplacd },
    { "setetop", "SETETOP", 2, c_setetop },
    { "sym2int", "SYM2INT", 1, c_sym2int },
    { "symbolp", "SYMBOLP", 1, c_symbolp },
    { "vectorp", "VECTORP", 1, c_vectorp },
    { "verbose", "VERBOSE", 1, c_verbose },
    { "vlen", "VLEN", 1, c_vlen },
    { "vref", "VREF", 2, c_vref },
    { "vset", "VSET", 3, c_vset },
    { "writereg", "WRITEREG", 2, c_writereg },
    { "zerop", "ZEROP", 1, c_zerop },
    { "~", "INOT", 1, c_inot },
    { ">>", "ASR", 2, c_asr },
    { "<<", "SHL", 2, c_shl }
};

/* Is this a primitive function name/code ? */
static PFUNC *ispcmd(DCELL *c)
{
    int i;

    i = 0;
    while (i < (sizeof(pftbl)/sizeof(PFUNC))) {
        if (c == (DCELL *)pftbl[i].fcode) {
            return &pftbl[i];
        }
        ++i;
    }

    return NULL;
}
static PFUNC *ispnam(DCELL *x)
{
    int i;

    i = 0;
    while (i < (sizeof(pftbl)/sizeof(PFUNC))) {
        if (EQNAM(x, pftbl[i].pname)) {
            return &pftbl[i];
        }
        ++i;
    }

    return NULL;
}

/* Print primitive function commands */
static void pcmdprint(DCELL *c)
{
    int i;

    i = 0;
    while (i < (sizeof(pftbl)/sizeof(PFUNC))) {
        if (c == (DCELL *)pftbl[i].fcode) {
            OPUTS(pftbl[i].cname);
            return;
        }
        ++i;
    }
}

/******************************************************************************/
/* Lisp interpreter                                                           */
/******************************************************************************/

int main(int arc, char *argv[])
{
    /* Initialize */
    init_platform();
    init_data();
    init_read();
    init_eval();

    /* Read, evaluate, print loop */
    done = 0;
    SETJMP(replenv);
    hookbrk(brkhand);
    while (!done) {
        read();
        eval();
        print();
    }

    /* Terminate */
    term_read();
    term_data();

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                  TERMS OF USE: MIT License
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
