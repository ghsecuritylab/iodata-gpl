/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <memory.h> /* for memset */
#include "conftest.h"
#include <string.h> /* for memset() */
#include <asm/types.h>

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

fhstatus *
mountproc_mnt_2(dirpath *argp, CLIENT *clnt)
{
	static fhstatus clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, MOUNTPROC_MNT,
		(xdrproc_t) xdr_dirpath, (caddr_t) argp,
		(xdrproc_t) xdr_fhstatus, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}