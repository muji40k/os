/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _BAKERY_H_RPCGEN
#define _BAKERY_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif

#define REGISTER 0
#define ACCESS 1
#define GET 2
#define STATUS 3
#define OP_MAX 4
#define STATUS_FREE 0
#define STATUS_REGISTERED 1
#define STATUS_ACCESSING 2
#define STATUS_READY_FOR_CR 3
#define ERROR_WRONG_ID_RPC -1
#define ERROR_INCOMPATIBLE_HANLE_RPC -2
#define ERROR_WRONG_STATUS_RPC -3
#define ERROR_REJECT_ACCESS_RPC -4
#define ERROR_WRONG_OP_RPC -5

struct BAKERY {
	int op;
	int id;
	int num;
	int result;
};
typedef struct BAKERY BAKERY;

#define BAKERY_PROG 0x20000001
#define BAKERY_VER 1

#if defined(__STDC__) || defined(__cplusplus)
#define BAKERY_PROC 1
extern  struct BAKERY * bakery_proc_1(struct BAKERY *, CLIENT *);
extern  struct BAKERY * bakery_proc_1_svc(struct BAKERY *, struct svc_req *);
extern int bakery_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define BAKERY_PROC 1
extern  struct BAKERY * bakery_proc_1();
extern  struct BAKERY * bakery_proc_1_svc();
extern int bakery_prog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_BAKERY (XDR *, BAKERY*);

#else /* K&R C */
extern bool_t xdr_BAKERY ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_BAKERY_H_RPCGEN */
