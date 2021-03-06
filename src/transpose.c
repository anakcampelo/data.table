#include "data.table.h"
#include <Rdefines.h>
#include <time.h>

SEXP transpose(SEXP l, SEXP fill, SEXP ignoreArg) {

    R_len_t i, j, k=0, ln, *len, maxlen=0, zerolen=0, anslen;
    SEXP li, thisi, ans;
    SEXPTYPE type, maxtype=0;
    Rboolean ignore, coerce = FALSE;

    if (!isNewList(l)) 
        error("l must be a list.");
    if (!length(l))
        return(duplicate(l));
    if (!isLogical(ignoreArg) || LOGICAL(ignoreArg)[0] == NA_LOGICAL)
        error("ignore.empty should be logical TRUE/FALSE.");
    if (length(fill) != 1)
        error("fill must be NULL or length=1 vector.");
    ln = LENGTH(l);
    ignore = LOGICAL(ignoreArg)[0];

    // preprocessing
    len  = Calloc(ln, R_len_t);
    for (i=0; i<ln; i++) {
        li = VECTOR_ELT(l, i);
        if (!isVectorAtomic(li)) 
            error("Item %d of list input is not an atomic vector", i+1);
        len[i] = length(li);
        if (len[i] > maxlen)
            maxlen = len[i];
        zerolen += (len[i] == 0);
        if (isFactor(li)) {
            maxtype = STRSXP;
        } else {
            type = TYPEOF(li);
            if (type > maxtype)
                maxtype = type;
        }
    }
    // coerce fill to maxtype
    fill = PROTECT(coerceVector(fill, maxtype));

    // allocate 'ans'
    ans = PROTECT(allocVector(VECSXP, maxlen));
    anslen = (!ignore) ? ln : (ln - zerolen);
    for (i=0; i<maxlen; i++) {
        thisi = allocVector(maxtype, anslen);
        SET_VECTOR_ELT(ans, i, thisi);
    }

    // transpose
    for (i=0; i<ln; i++) {
        if (ignore && !len[i]) continue;
        li = VECTOR_ELT(l, i);
        if (TYPEOF(li) != maxtype) {
            coerce = TRUE;
            if (!isFactor(li)) li = PROTECT(coerceVector(li, maxtype));
            else li = PROTECT(asCharacterFactor(li));
        }
        switch (maxtype) {
            case INTSXP :
                for (j=0; j<maxlen; j++) {
                    thisi = VECTOR_ELT(ans, j);
                    INTEGER(thisi)[k] = (j < len[i]) ? INTEGER(li)[j] : INTEGER(fill)[0];
                }
            break;
            case LGLSXP :
                for (j=0; j<maxlen; j++) {
                    thisi = VECTOR_ELT(ans, j);
                    LOGICAL(thisi)[k] = (j < len[i]) ? LOGICAL(li)[j] : LOGICAL(fill)[0];
                }
            break;
            case REALSXP :
                for (j=0; j<maxlen; j++) {
                    thisi = VECTOR_ELT(ans, j);
                    REAL(thisi)[k] = (j < len[i]) ? REAL(li)[j] : REAL(fill)[0];
                }
            break;
            case STRSXP :
                for (j=0; j<maxlen; j++) {
                    thisi = VECTOR_ELT(ans, j);
                    SET_STRING_ELT(thisi, k, (j < len[i]) ? STRING_ELT(li, j) : STRING_ELT(fill, 0));
                }
            break;
            default :
                error("Unsupported column type '%s'", type2char(maxtype)); 
        }
        if (coerce) {
            coerce = FALSE;
            UNPROTECT(1);
        }
        k++;
    }
    Free(len);
    UNPROTECT(2);
    return(ans);
}
