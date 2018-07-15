/*
 * This source file is part of the bstring string library.  This code was
 * written by Paul Hsieh in 2002-2015, and is covered by the BSD open source
 * license and the GPL. Refer to the accompanying documentation for details
 * on usage and license.
 */

/**
 * @file
 * @brief This file is the interface for the core bstring functions.
 */

#ifndef BSTRLIB_INCLUDE
#define BSTRLIB_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __palmos__
#include <string.h>
#include <ctype.h>
#include <assert.h>
#endif /* __palmos__ */
#include <stdarg.h>
#include <limits.h>

#if !defined (BSTRLIB_VSNP_OK) && !defined (BSTRLIB_NOVSNP)
# if defined (__TURBOC__) && !defined (__BORLANDC__)
#  define BSTRLIB_NOVSNP
# endif
#endif

#ifdef USE_BSTRING_LISTS
#ifndef USE_BSTRING_CALLBACKS
#error bstring callbacks must be enabled to use bstring lists!
#endif /* USE_BSTRING_CALLBACKS */
#endif /* USE_BSTRING_LISTS */

#define BSTR_ERR (-1)
#define BSTR_OK (0)
#define BSTR_BS_BUFF_LENGTH_GET (0)

typedef struct tagbstring * bstring;
typedef const struct tagbstring * const_bstring;

/* Version */
#define BSTR_VER_MAJOR  1
#define BSTR_VER_MINOR  0
#define BSTR_VER_UPDATE 0

/* Copy functions */
#define cstr2bstr bfromcstr
extern bstring bfromcstr (const char * str);
extern bstring bfromcstralloc (int mlen, const char * str);
extern bstring bfromcstrrangealloc (int minl, int maxl, const char* str);
extern bstring blk2bstr (const void * blk, int len);
extern char * bstr2cstr (const_bstring s, char z);
extern int bcstrfree (char * s);
extern bstring bstrcpy (const_bstring b1);
extern int bassign (bstring a, const_bstring b)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
extern int bassignmidstr (bstring a, const_bstring b, int left, int len)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
extern int bassigncstr (bstring a, const char * str)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
extern int bassignblk (bstring a, const void * s, int len)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;

/* Destroy function */
extern int bdestroy (bstring b);

/* Space allocation hinting functions */
extern int balloc (bstring s, int len);
extern int ballocmin (bstring b, int len);

/* Substring extraction */
extern bstring bmidstr (const_bstring b, int left, int len);

/* Various standard manipulations */
extern int bconcat (bstring b0, const_bstring b1)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
extern int bconchar (bstring b0, char c)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
extern int bcatcstr (bstring b, const char * s)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
extern int bcatblk (bstring b, const void * s, int len)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
extern int binsert (bstring s1, int pos, const_bstring s2, unsigned char fill)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
extern int binsertblk (bstring s1, int pos, const void * s2, int len, unsigned char fill)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
extern int binsertch (bstring s1, int pos, int len, unsigned char fill)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
extern int breplace (bstring b1, int pos, int len, const_bstring b2, unsigned char fill)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
extern int bdelete (bstring s1, int pos, int len);
extern int bsetstr (bstring b0, int pos, const_bstring b1, unsigned char fill)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
extern int btrunc (bstring b, int n)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;

/* Scan/search functions */
extern int bstricmp (const_bstring b0, const_bstring b1);
extern int bstrnicmp (const_bstring b0, const_bstring b1, int n);
extern int biseqcaseless (const_bstring b0, const_bstring b1);
extern int biseqcaselessblk (const_bstring b, const void * blk, int len);
extern int bisstemeqcaselessblk (const_bstring b0, const void * blk, int len);
extern int biseq (const_bstring b0, const_bstring b1);
extern int biseqblk (const_bstring b, const void * blk, int len);
extern int bisstemeqblk (const_bstring b0, const void * blk, int len);
extern int biseqcstr (const_bstring b, const char * s);
extern int biseqcstrcaseless (const_bstring b, const char * s);
extern int bstrcmp (const_bstring b0, const_bstring b1);
extern int bstrncmp (const_bstring b0, const_bstring b1, int n);
extern int binstr (const_bstring s1, int pos, const_bstring s2);
extern int binstrr (const_bstring s1, int pos, const_bstring s2);
extern int binstrcaseless (const_bstring s1, int pos, const_bstring s2);
extern int binstrrcaseless (const_bstring s1, int pos, const_bstring s2);
extern int bstrchrp (const_bstring b, int c, int pos);
extern int bstrrchrp (const_bstring b, int c, int pos);
/**
 * \brief   Search for the character c in the bstring b forwards from the
 *          start of the bstring. Returns the position of the found character
 *          or BSTR_ERR if it is not found.
 *
 *          NOTE: This has been implemented as a macro on top of bstrchrp().
 */
#define bstrchr(b,c) bstrchrp ((b), (c), 0)
/**
 * \brief   Search for the character c in the bstring b backwards from the
 *          end of the bstring. Returns the position of the found character
 *          or BSTR_ERR if it is not found.
 *
 *          NOTE: This has been implemented as a macro on top of bstrrchrp().
 */
#define bstrrchr(b,c) bstrrchrp ((b), (c), blength(b)-1)
extern int binchr (const_bstring b0, int pos, const_bstring b1);
extern int binchrr (const_bstring b0, int pos, const_bstring b1);
extern int bninchr (const_bstring b0, int pos, const_bstring b1);
extern int bninchrr (const_bstring b0, int pos, const_bstring b1);
extern int bfindreplace (bstring b, const_bstring find, const_bstring repl, int pos);
extern int bfindreplacecaseless (bstring b, const_bstring find, const_bstring repl, int pos);

#ifdef USE_BSTRING_LISTS

/* List of string container functions */
struct bstrList {
    int qty, mlen;
    bstring * entry;
};
extern struct bstrList * bstrListCreate (void);
extern int bstrListDestroy (struct bstrList * sl);
extern int bstrListAlloc (struct bstrList * sl, int msz);
extern int bstrListAllocMin (struct bstrList * sl, int msz);

/* String split and join functions */
extern struct bstrList * bsplit (const_bstring str, unsigned char splitChar);
extern struct bstrList * bsplits (const_bstring str, const_bstring splitStr);
extern struct bstrList * bsplitstr (const_bstring str, const_bstring splitStr);
extern bstring bjoin (const struct bstrList * bl, const_bstring sep);
extern bstring bjoinblk (const struct bstrList * bl, const void * s, int len);

#endif /* USE_BSTRING_LISTS */

#ifdef USE_BSTRING_CALLBACKS

extern int bsplitcb (const_bstring str, unsigned char splitChar, int pos,
	int (* cb) (void * parm, int ofs, int len), void * parm);
extern int bsplitscb (const_bstring str, const_bstring splitStr, int pos,
	int (* cb) (void * parm, int ofs, int len), void * parm);
extern int bsplitstrcb (const_bstring str, const_bstring splitStr, int pos,
	int (* cb) (void * parm, int ofs, int len), void * parm);

#endif /* USE_BSTRING_CALLBACKS */

/* Miscellaneous functions */
extern int bpattern (bstring b, int len);
extern int btoupper (bstring b);
extern int btolower (bstring b);
extern int bltrimws (bstring b);
extern int brtrimws (bstring b);
extern int btrimws (bstring b);

#if !defined (BSTRLIB_NOVSNP)
extern bstring bformat (const char * fmt, ...);
extern int bformata (bstring b, const char * fmt, ...)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
extern int bassignformat (bstring b, const char * fmt, ...)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
extern int bvcformata (bstring b, int count, const char * fmt, va_list arglist)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;

#define bvformata(ret, b, fmt, lastarg) { \
bstring bstrtmp_b = (b); \
const char * bstrtmp_fmt = (fmt); \
int bstrtmp_r = BSTR_ERR, bstrtmp_sz = 16; \
	for (;;) { \
		va_list bstrtmp_arglist; \
		va_start (bstrtmp_arglist, lastarg); \
		bstrtmp_r = bvcformata (bstrtmp_b, bstrtmp_sz, bstrtmp_fmt, bstrtmp_arglist); \
		va_end (bstrtmp_arglist); \
		if (bstrtmp_r >= 0) { /* Everything went ok */ \
			bstrtmp_r = BSTR_OK; \
			break; \
		} else if (-bstrtmp_r <= bstrtmp_sz) { /* A real error? */ \
			bstrtmp_r = BSTR_ERR; \
			break; \
		} \
		bstrtmp_sz = -bstrtmp_r; /* Doubled or target size */ \
	} \
	ret = bstrtmp_r; \
}

#endif

#ifdef USE_BSTRING_STREAMS

typedef int (*bNgetc) (void *parm);
typedef size_t (* bNread) (void *buff, size_t elsize, size_t nelem, void *parm);

/* Input functions */
extern bstring bgets (bNgetc getcPtr, void * parm, char terminator);
extern bstring bread (bNread readPtr, void * parm);
extern int bgetsa (bstring b, bNgetc getcPtr, void * parm, char terminator);
extern int bassigngets (bstring b, bNgetc getcPtr, void * parm, char terminator);
extern int breada (bstring b, bNread readPtr, void * parm);

/* Stream functions */
extern struct bStream * bsopen (bNread readPtr, void * parm);
extern void * bsclose (struct bStream * s);
extern int bsbufflength (struct bStream * s, int sz);
extern int bsreadln (bstring b, struct bStream * s, char terminator);
extern int bsreadlns (bstring r, struct bStream * s, const_bstring term);
extern int bsread (bstring b, struct bStream * s, int n);
extern int bsreadlna (bstring b, struct bStream * s, char terminator);
extern int bsreadlnsa (bstring r, struct bStream * s, const_bstring term);
extern int bsreada (bstring b, struct bStream * s, int n);
extern int bsunread (struct bStream * s, const_bstring b);
extern int bspeek (bstring r, const struct bStream * s);
extern int bssplitscb (struct bStream * s, const_bstring splitStr,
	int (* cb) (void * parm, int ofs, const_bstring entry), void * parm);
extern int bssplitstrcb (struct bStream * s, const_bstring splitStr,
	int (* cb) (void * parm, int ofs, const_bstring entry), void * parm);
extern int bseof (const struct bStream * s);

#endif /* USE_BSTRING_STREAMS */

struct tagbstring {
	int mlen;
	int slen;
	unsigned char * data;
};

/* Accessor macros */
#define blengthe(b, e)      (((b) == (void *)0 || (b)->slen < 0) ? (int)(e) : ((b)->slen))
/**
 * \brief   Returns the length of the bstring.  If the bstring is NULL, the
 *          length returned is 0.
 */
#define blength(b)          (blengthe ((b), 0))
#define bdataofse(b, o, e)  (((b) == (void *)0 || (b)->data == (void*)0) ? (char *)(e) : ((char *)(b)->data) + (o))
#define bdataofs(b, o)      (bdataofse ((b), (o), (void *)0))
#define bdatae(b, e)        (bdataofse (b, 0, e))
#define bdata(b)            (bdataofs (b, 0))
#define bchare(b, p, e)     ((((unsigned)(p)) < (unsigned)blength(b)) ? ((b)->data[(p)]) : (e))
#define bchar(b, p)         bchare ((b), (p), '\0')

/* Static constant string initialization macro */
#define bsStaticMlen(q,m)   {(m), (int) sizeof(q)-1, (unsigned char *) ("" q "")}
#if defined(_MSC_VER)
# define bsStatic(q)        bsStaticMlen(q,-32)
#endif
#ifndef bsStatic
# define bsStatic(q)        bsStaticMlen(q,-__LINE__)
#endif

/* Static constant block parameter pair */
/**
 * \brief   The bsStaticBlkParms macro emits a pair of comma seperated
 *          parameters corresponding to the block parameters for the block
 *          functions in Bstrlib (i.e., blk2bstr(), bcatblk(), blk2tbstr(),
 *          bisstemeqblk(), bisstemeqcaselessblk().)
 *          Note that this macro is only well defined for string literal
 *          arguments.
 *
 *          Examples:
 *
 *          bstring b = blk2bstr(bsStaticBlkParms("Fast init. "));
 *
 *          bcatblk(b, bsStaticBlkParms("No frills fast concatenation."));
 *
 *          These are faster than using bfromcstr() and bcatcstr()
 *          respectively because the length of the inline string is known as
 *          a compile time constant. Also note that seperate struct tagbstring
 *          declarations for holding the output of a bsStatic() macro are not
 *          required.
 */
#define bsStaticBlkParms(q) ((void *)("" q "")), ((int) sizeof(q)-1)
/**
 * \brief   Append a string literal to bstring b.  Returns 0 if successful, or
 *          BSTR_ERR if some error has occurred.  The string literal parameter
 *          is enforced as literal at compile time.
 */
#define bcatStatic(b,s)     ((bcatblk)((b), bsStaticBlkParms(s)))
/**
 * \brief   Allocate a bstring with the contents of a string literal. Returns
 *          NULL if an error has occurred (ran out of memory). The string
 *          literal parameter is enforced as literal at compile time.
 */
#define bfromStatic(s)      ((blk2bstr)(bsStaticBlkParms(s)))
/**
 * \brief   Assign the contents of a string literal to the bstring b. The
 *          string literal parameter is enforced as literal at compile time.
 */
#define bassignStatic(b,s)  ((bassignblk)((b), bsStaticBlkParms(s)))
/**
 * \brief   Inserts the string literal into s1 at position pos. If the
 *          position pos is past the end of s1, then the character "fill"
 *          is appended as necessary to make up the gap between the end of s1
 *          and pos. The value BSTR_OK is returned if the operation is
 *          successful, otherwise BSTR_ERR is returned.
 */
#define binsertStatic(b,p,s,f) ((binsertblk)((b), (p), bsStaticBlkParms(s), (f)))
/**
 * \brief   Join the entries of a bstrList into one bstring by sequentially
 *          concatenating them with the string literal in between. If there
 *          is an error NULL is returned, otherwise a bstring with the correct
 *          result is returned. See bstrListCreate() above for structure of
 *          struct bstrList.
 */
#define bjoinStatic(b,s)    ((bjoinblk)((b), bsStaticBlkParms(s)))
/**
 * \brief   Compare the string b with the string literal. If the content
 *          differs, 0 is returned, if the content is the same, 1 is returned,
 *          if there is an error, -1 is returned.  If the length of the
 *          strings are different, this function is O(1). '\0' characters
 *          are not treated in any special way.
 */
#define biseqStatic(b,s)    ((biseqblk)((b), bsStaticBlkParms(s)))
/**
 * \brief   Compare beginning of bstring b with a string literal for equality.
 *          If the beginning of b differs from the memory block (or if b is
 *          too short), 0 is returned, if the bstrings are the same, 1 is
 *          returned, if there is an error, -1 is returned. The string literal
 *          parameter is enforced as literal at compile time.
 */
#define bisstemeqStatic(b,s) ((bisstemeqblk)((b), bsStaticBlkParms(s)))
#define biseqcaselessStatic(b,s) ((biseqcaselessblk)((b), bsStaticBlkParms(s)))
#define bisstemeqcaselessStatic(b,s) ((bisstemeqcaselessblk)((b), bsStaticBlkParms(s)))

/* Reference building macros */
#define cstr2tbstr btfromcstr
#define btfromcstr(t,s) {                                            \
    (t).data = (unsigned char *) (s);                                \
    (t).slen = ((t).data) ? ((int) (strlen) ((char *)(t).data)) : 0; \
    (t).mlen = -1;                                                   \
}
#define blk2tbstr(t,s,l) {            \
    (t).data = (unsigned char *) (s); \
    (t).slen = l;                     \
    (t).mlen = -1;                    \
}
#define btfromblk(t,s,l) blk2tbstr(t,s,l)
#define bmid2tbstr(t,b,p,l) {                                                \
    const_bstring bstrtmp_s = (b);                                           \
    if (bstrtmp_s && bstrtmp_s->data && bstrtmp_s->slen >= 0) {              \
        int bstrtmp_left = (p);                                              \
        int bstrtmp_len  = (l);                                              \
        if (bstrtmp_left < 0) {                                              \
            bstrtmp_len += bstrtmp_left;                                     \
            bstrtmp_left = 0;                                                \
        }                                                                    \
        if (bstrtmp_len > bstrtmp_s->slen - bstrtmp_left)                    \
            bstrtmp_len = bstrtmp_s->slen - bstrtmp_left;                    \
        if (bstrtmp_len <= 0) {                                              \
            (t).data = (unsigned char *)"";                                  \
            (t).slen = 0;                                                    \
        } else {                                                             \
            (t).data = bstrtmp_s->data + bstrtmp_left;                       \
            (t).slen = bstrtmp_len;                                          \
        }                                                                    \
    } else {                                                                 \
        (t).data = (unsigned char *)"";                                      \
        (t).slen = 0;                                                        \
    }                                                                        \
    (t).mlen = -__LINE__;                                                    \
}
#define btfromblkltrimws(t,s,l) {                                            \
    int bstrtmp_idx = 0, bstrtmp_len = (l);                                  \
    unsigned char * bstrtmp_s = (s);                                         \
    if (bstrtmp_s && bstrtmp_len >= 0) {                                     \
        for (; bstrtmp_idx < bstrtmp_len; bstrtmp_idx++) {                   \
            if (!isspace (bstrtmp_s[bstrtmp_idx])) break;                    \
        }                                                                    \
    }                                                                        \
    (t).data = bstrtmp_s + bstrtmp_idx;                                      \
    (t).slen = bstrtmp_len - bstrtmp_idx;                                    \
    (t).mlen = -__LINE__;                                                    \
}
#define btfromblkrtrimws(t,s,l) {                                            \
    int bstrtmp_len = (l) - 1;                                               \
    unsigned char * bstrtmp_s = (s);                                         \
    if (bstrtmp_s && bstrtmp_len >= 0) {                                     \
        for (; bstrtmp_len >= 0; bstrtmp_len--) {                            \
            if (!isspace (bstrtmp_s[bstrtmp_len])) break;                    \
        }                                                                    \
    }                                                                        \
    (t).data = bstrtmp_s;                                                    \
    (t).slen = bstrtmp_len + 1;                                              \
    (t).mlen = -__LINE__;                                                    \
}
#define btfromblktrimws(t,s,l) {                                             \
    int bstrtmp_idx = 0, bstrtmp_len = (l) - 1;                              \
    unsigned char * bstrtmp_s = (s);                                         \
    if (bstrtmp_s && bstrtmp_len >= 0) {                                     \
        for (; bstrtmp_idx <= bstrtmp_len; bstrtmp_idx++) {                  \
            if (!isspace (bstrtmp_s[bstrtmp_idx])) break;                    \
        }                                                                    \
        for (; bstrtmp_len >= bstrtmp_idx; bstrtmp_len--) {                  \
            if (!isspace (bstrtmp_s[bstrtmp_len])) break;                    \
        }                                                                    \
    }                                                                        \
    (t).data = bstrtmp_s + bstrtmp_idx;                                      \
    (t).slen = bstrtmp_len + 1 - bstrtmp_idx;                                \
    (t).mlen = -__LINE__;                                                    \
}

/* Write protection macros */
/**
 * \brief   Disallow bstring from being written to via the bstrlib API.
 *          Attempts to write to the resulting tagbstring from any bstrlib
 *          function will lead to BSTR_ERR being returned.
 *
 *          Note: bstrings which are write protected cannot be destroyed via
 *          bdestroy.
 *
 *          Note to C++ users: Setting a CBString as write protected will not
 *          prevent it from being destroyed by the destructor.
 */
#define bwriteprotect(t)     { if ((t).mlen >=  0) (t).mlen = -1; }
/**
 * \brief   Allow bstring to be written to via the bstrlib API. Note that such
 *          an action makes the bstring both writable and destroyable. If the
 *          bstring is not legitimately writable (as is the case for struct
 *          tagbstrings initialized with a bsStatic value), the results of
 *          this are undefined.
 *
 *          Note that invoking the bwriteallow macro may increase the number
 *          of reallocs by one more than necessary for every call to
 *          bwriteallow interleaved with any bstring API which writes to this
 *          bstring.
 */
#define bwriteallow(t)       { if ((t).mlen == -1) (t).mlen = (t).slen + ((t).slen == 0); }
/**
 * \brief   Returns 1 if the bstring is write protected, otherwise 0 is
 *          returned.
 */
#define biswriteprotected(t) ((t).mlen <= 0)

#ifdef __cplusplus
}
#endif

#endif
