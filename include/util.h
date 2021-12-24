/**
 * \file
 *	Utilities
 *
 * \author
 *	Kadir Yanık <kdrynkk@gmail.com>
 *
 * \date
 *	12.2021
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <stdlib.h>
#include <stdint.h>

/*------------------------------------------------------------------------------*/
#define DGREEN	"\033[32;2m"
#define LRED	"\033[31m"
#define LBLUE	"\033[34m"
#define NORM	"\033[0m"

#define VA_ARGS(...) , ##__VA_ARGS__
#define LOG_ERR(frm, ...) fprintf(stderr, LRED "%s[%d]: " NORM frm, __func__, __LINE__ VA_ARGS(__VA_ARGS__));
#define LOG_INFO(...) printf(__VA_ARGS__);
#define LOG_DEBUG(frm, ...) printf(LBLUE "%s[%d]: " NORM frm, __func__, __LINE__ VA_ARGS(__VA_ARGS__));

/*------------------------------------------------------------------------------*/
#define sfree(_p) do {	    \
	if ((_p)) {	    \
	    free((_p));	    \
	    (_p) = NULL;    \
	}		    \
    } while (0)

/*------------------------------------------------------------------------------*/
/* Goto fail in error with expression */
#define util_fiee(f,e) do { \
	if ((f) != 0) {	    \
	    e;		    \
	    goto fail;	    \
	}		    \
    } while (0)

#define util_fie(f) util_fiee(f,{ do { } while (0); })

/*------------------------------------------------------------------------------*/
/* Goto fail/success in condition true with expression */
#define util_xite(c,l,e) do {	\
	if (c) {		\
	    e;			\
	    goto l;		\
	}			\
    } while (0)

/* fail in true w/o expression */
#define util_fit(c) util_xite(c,fail,{ do { } while (0); })
#define util_fite(c,e) util_xite(c,fail,e)
/* success in true w/o expression */
#define util_sit(c) util_xite(c,success,{ do { } while (0); })
#define util_site(c,e) util_xite(c,success,e)

#endif /* UTIL_H_ */
