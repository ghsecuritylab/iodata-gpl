/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _CONFTEST_H_RPCGEN
#define _CONFTEST_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif

#include <asm/types.h>

typedef char fhandle[1];

struct fhstatus {
	u_int fhs_status;
	union {
		fhandle fhs_fhandle;
	} fhstatus_u;
};
typedef struct fhstatus fhstatus;

typedef char *dirpath;

struct ppathcnf {
	short pc_mask[2];
};
typedef struct ppathcnf ppathcnf;

#define MOUNTPROG 100005
#define MOUNTVERS 2

#if defined(__STDC__) || defined(__cplusplus)
#define MOUNTPROC_MNT 1
extern  fhstatus * mountproc_mnt_2(dirpath *, CLIENT *);
extern  fhstatus * mountproc_mnt_2_svc(dirpath *, struct svc_req *);
extern int mountprog_2_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define MOUNTPROC_MNT 1
extern  fhstatus * mountproc_mnt_2();
extern  fhstatus * mountproc_mnt_2_svc();
extern int mountprog_2_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_fhandle (XDR *, fhandle);
extern  bool_t xdr_fhstatus (XDR *, fhstatus*);
extern  bool_t xdr_dirpath (XDR *, dirpath*);
extern  bool_t xdr_ppathcnf (XDR *, ppathcnf*);

#else /* K&R C */
extern bool_t xdr_fhandle ();
extern bool_t xdr_fhstatus ();
extern bool_t xdr_dirpath ();
extern bool_t xdr_ppathcnf ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_CONFTEST_H_RPCGEN */
