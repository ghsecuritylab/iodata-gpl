
#ifdef RPC_CLNT
%#include <string.h>            /* for memset() */
#endif
%#include <asm/types.h>
typedef opaque fhandle[1];
union fhstatus switch (unsigned fhs_status) {
case 0:
        fhandle fhs_fhandle;
default:
        void;
};
typedef string dirpath<1024>;
struct ppathcnf {
    short   pc_mask[2];
};

program MOUNTPROG {
    version MOUNTVERS {
	fhstatus
	MOUNTPROC_MNT(dirpath) = 1;
    } = 2;
} = 100005;

