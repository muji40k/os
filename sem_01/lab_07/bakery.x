const REGISTER = 0;
const ACCESS   = 1;
const GET      = 2;
const STATUS   = 3;
const OP_MAX   = 4;

const STATUS_FREE         = 0;
const STATUS_REGISTERED   = 1;
const STATUS_ACCESSING    = 2;
const STATUS_READY_FOR_CR = 3;

const ERROR_WRONG_ID_RPC           = -1;
const ERROR_INCOMPATIBLE_HANLE_RPC = -2;
const ERROR_WRONG_STATUS_RPC       = -3;
const ERROR_REJECT_ACCESS_RPC      = -4;
const ERROR_WRONG_OP_RPC           = -5;

struct BAKERY
{
    int op;
    int id;
    int num;
    int result;
};

program BAKERY_PROG
{
    version BAKERY_VER
    {
        struct BAKERY BAKERY_PROC(struct BAKERY) = 1;
    } = 1;
} = 0x20000001;

