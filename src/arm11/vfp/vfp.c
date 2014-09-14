/*
    armvfp.c - ARM VFPv3 emulation unit
    Copyright (C) 2003 Skyeye Develop Group
    for help please send mail to <skyeye-developer@lists.gro.clinux.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* Note: this file handles interface with arm core and vfp registers */

/* Opens debug for classic interpreter only */
//#define DEBUG

#include "../armdefs.h"
#include "vfp.h"

//ARMul_State* persistent_state; /* function calls from SoftFloat lib don't have an access to ARMul_state. */

unsigned
VFPInit (ARMul_State *state)
{
    state->VFP[VFP_OFFSET(VFP_FPSID)] = VFP_FPSID_IMPLMEN<<24 | VFP_FPSID_SW<<23 | VFP_FPSID_SUBARCH<<16 |
                                        VFP_FPSID_PARTNUM<<8 | VFP_FPSID_VARIANT<<4 | VFP_FPSID_REVISION;
    state->VFP[VFP_OFFSET(VFP_FPEXC)] = 0;
    state->VFP[VFP_OFFSET(VFP_FPSCR)] = 0;

    //persistent_state = state;
    /* Reset only specify VFP_FPEXC_EN = '0' */

    return 0;
}

unsigned
VFPMRC (ARMul_State * state, unsigned type, u32 instr, u32 * value)
{
    /* MRC<c> <coproc>,<opc1>,<Rt>,<CRn>,<CRm>{,<opc2>} */
    int CoProc = BITS (8, 11); /* 10 or 11 */
    int OPC_1 = BITS (21, 23);
    int Rt = BITS (12, 15);
    int CRn = BITS (16, 19);
    int CRm = BITS (0, 3);
    int OPC_2 = BITS (5, 7);

    /* TODO check access permission */

    /* CRn/opc1 CRm/opc2 */

    if (CoProc == 10 || CoProc == 11) {
#define VFP_MRC_TRANS
#include "vfpinstr.c"
#undef VFP_MRC_TRANS
    }
    DEBUG("Can't identify %x, CoProc %x, OPC_1 %x, Rt %x, CRn %x, CRm %x, OPC_2 %x\n",
          instr, CoProc, OPC_1, Rt, CRn, CRm, OPC_2);

    return ARMul_CANT;
}

unsigned
VFPMCR (ARMul_State * state, unsigned type, u32 instr, u32 value)
{
    /* MCR<c> <coproc>,<opc1>,<Rt>,<CRn>,<CRm>{,<opc2>} */
    int CoProc = BITS (8, 11); /* 10 or 11 */
    int OPC_1 = BITS (21, 23);
    int Rt = BITS (12, 15);
    int CRn = BITS (16, 19);
    int CRm = BITS (0, 3);
    int OPC_2 = BITS (5, 7);

    /* TODO check access permission */

    /* CRn/opc1 CRm/opc2 */
    if (CoProc == 10 || CoProc == 11) {
#define VFP_MCR_TRANS
#include "vfpinstr.c"
#undef VFP_MCR_TRANS
    }
    DEBUG("Can't identify %x, CoProc %x, OPC_1 %x, Rt %x, CRn %x, CRm %x, OPC_2 %x\n",
          instr, CoProc, OPC_1, Rt, CRn, CRm, OPC_2);

    return ARMul_CANT;
}

unsigned
VFPMRRC (ARMul_State * state, unsigned type, u32 instr, u32 * value1, u32 * value2)
{
    /* MCRR<c> <coproc>,<opc1>,<Rt>,<Rt2>,<CRm> */
    int CoProc = BITS (8, 11); /* 10 or 11 */
    int OPC_1 = BITS (4, 7);
    int Rt = BITS (12, 15);
    int Rt2 = BITS (16, 19);
    int CRm = BITS (0, 3);

    if (CoProc == 10 || CoProc == 11) {
#define VFP_MRRC_TRANS
#include "vfpinstr.c"
#undef VFP_MRRC_TRANS
    }
    DEBUG("Can't identify %x, CoProc %x, OPC_1 %x, Rt %x, Rt2 %x, CRm %x\n",
          instr, CoProc, OPC_1, Rt, Rt2, CRm);

    return ARMul_CANT;
}

unsigned
VFPMCRR (ARMul_State * state, unsigned type, u32 instr, u32 value1, u32 value2)
{
    /* MCRR<c> <coproc>,<opc1>,<Rt>,<Rt2>,<CRm> */
    int CoProc = BITS (8, 11); /* 10 or 11 */
    int OPC_1 = BITS (4, 7);
    int Rt = BITS (12, 15);
    int Rt2 = BITS (16, 19);
    int CRm = BITS (0, 3);

    /* TODO check access permission */

    /* CRn/opc1 CRm/opc2 */

    if (CoProc == 11 || CoProc == 10) {
#define VFP_MCRR_TRANS
#include "vfpinstr.c"
#undef VFP_MCRR_TRANS
    }
    DEBUG("Can't identify %x, CoProc %x, OPC_1 %x, Rt %x, Rt2 %x, CRm %x\n",
          instr, CoProc, OPC_1, Rt, Rt2, CRm);

    return ARMul_CANT;
}

unsigned
VFPSTC (ARMul_State * state, unsigned type, u32 instr, u32 * value)
{
    /* STC{L}<c> <coproc>,<CRd>,[<Rn>],<option> */
    int CoProc = BITS (8, 11); /* 10 or 11 */
    int CRd = BITS (12, 15);
    int Rn = BITS (16, 19);
    int imm8 = BITS (0, 7);
    int P = BIT(24);
    int U = BIT(23);
    int D = BIT(22);
    int W = BIT(21);

    /* TODO check access permission */

    /* VSTM */
    if ( (P|U|D|W) == 0 ) {
        DEBUG("In %s, UNDEFINED\n", __FUNCTION__);
        exit(-1);
    }
    if (CoProc == 10 || CoProc == 11) {
#if 1
        if (P == 0 && U == 0 && W == 0) {
            DEBUG("VSTM Related encodings\n");
            exit(-1);
        }
        if (P == U && W == 1) {
            DEBUG("UNDEFINED\n");
            exit(-1);
        }
#endif

#define VFP_STC_TRANS
#include "vfpinstr.c"
#undef VFP_STC_TRANS
    }
    DEBUG("Can't identify %x, CoProc %x, CRd %x, Rn %x, imm8 %x, P %x, U %x, D %x, W %x\n",
          instr, CoProc, CRd, Rn, imm8, P, U, D, W);

    return ARMul_CANT;
}

unsigned
VFPLDC (ARMul_State * state, unsigned type, u32 instr, u32 value)
{
    /* LDC{L}<c> <coproc>,<CRd>,[<Rn>] */
    int CoProc = BITS (8, 11); /* 10 or 11 */
    int CRd = BITS (12, 15);
    int Rn = BITS (16, 19);
    int imm8 = BITS (0, 7);
    int P = BIT(24);
    int U = BIT(23);
    int D = BIT(22);
    int W = BIT(21);

    /* TODO check access permission */

    if ( (P|U|D|W) == 0 ) {
        DEBUG("In %s, UNDEFINED\n", __FUNCTION__);
        exit(-1);
    }
    if (CoProc == 10 || CoProc == 11) {
#define VFP_LDC_TRANS
#include "vfpinstr.c"
#undef VFP_LDC_TRANS
    }
    DEBUG("Can't identify %x, CoProc %x, CRd %x, Rn %x, imm8 %x, P %x, U %x, D %x, W %x\n",
          instr, CoProc, CRd, Rn, imm8, P, U, D, W);

    return ARMul_CANT;
}

unsigned
VFPCDP (ARMul_State * state, unsigned type, u32 instr)
{
    /* CDP<c> <coproc>,<opc1>,<CRd>,<CRn>,<CRm>,<opc2> */
    int CoProc = BITS (8, 11); /* 10 or 11 */
    int OPC_1 = BITS (20, 23);
    int CRd = BITS (12, 15);
    int CRn = BITS (16, 19);
    int CRm = BITS (0, 3);
    int OPC_2 = BITS (5, 7);

    //ichfly
    if ((instr & 0x0FBF0FD0) == 0x0EB70AC0) //vcvt.f64.f32	d8, s16 (s is bit 0-3 and LSB bit 22) (d is bit 12 - 15 MSB is Bit 6)
    {
        struct vfp_double vdd;
        struct vfp_single vsd;
        int dn = BITS(12, 15) + (BIT(22) << 4);
        int sd = (BITS(0, 3) << 1) + BIT(5);
        s32 n = vfp_get_float(state, sd);
        vfp_single_unpack(&vsd, n);
        if (vsd.exponent & 0x80)
        {
            vdd.exponent = (vsd.exponent&~0x80) | 0x400;
        }
        else
        {
            vdd.exponent = vsd.exponent | 0x380;
        }
        vdd.sign = vsd.sign;
        vdd.significand = (u64)(vsd.significand & ~0xC0000000) << 32; // I have no idea why but the 2 uppern bits are not from the significand
        vfp_put_double(state, vfp_double_pack(&vdd), dn);
        return ARMul_DONE;
    }
    if ((instr & 0x0FBF0FD0) == 0x0EB70BC0) //vcvt.f32.f64	s15, d6
    {
        struct vfp_double vdd;
        struct vfp_single vsd;
        int sd = BITS(0, 3) + (BIT(5) << 4);
        int dn = (BITS(12, 15) << 1) + BIT(22);
        vfp_double_unpack(&vdd, vfp_get_double(state, sd));
        if (vdd.exponent & 0x400) //todo if the exponent is to low or to high for this convert
        {
            vsd.exponent = (vdd.exponent) | 0x80;
        }
        else
        {
            vsd.exponent = vdd.exponent & ~0x80;
        }
        vsd.exponent &= 0xFF;
       // vsd.exponent = vdd.exponent >> 3;
        vsd.sign = vdd.sign;
        vsd.significand = ((u64)(vdd.significand ) >> 32)& ~0xC0000000;
        vfp_put_float(state, vfp_single_pack(&vsd), dn);
        return ARMul_DONE;
    }

    /* TODO check access permission */

    /* CRn/opc1 CRm/opc2 */

    if (CoProc == 10 || CoProc == 11) {
#define VFP_CDP_TRANS
#include "vfpinstr.c"
#undef VFP_CDP_TRANS

        int exceptions = 0;
        if (CoProc == 10)
            exceptions = vfp_single_cpdo(state, instr, state->VFP[VFP_OFFSET(VFP_FPSCR)]);
        else
            exceptions = vfp_double_cpdo(state, instr, state->VFP[VFP_OFFSET(VFP_FPSCR)]);

        vfp_raise_exceptions(state, exceptions, instr, state->VFP[VFP_OFFSET(VFP_FPSCR)]);

        return ARMul_DONE;
    }
    DEBUG("Can't identify %x\n", instr);
    return ARMul_CANT;
}


/* ----------- MRC ------------ */
#define VFP_MRC_IMPL
#include "vfpinstr.c"
#undef VFP_MRC_IMPL

#define VFP_MRRC_IMPL
#include "vfpinstr.c"
#undef VFP_MRRC_IMPL


/* ----------- MCR ------------ */
#define VFP_MCR_IMPL
#include "vfpinstr.c"
#undef VFP_MCR_IMPL

#define VFP_MCRR_IMPL
#include "vfpinstr.c"
#undef VFP_MCRR_IMPL

/* Memory operation are not inlined, as old Interpreter and Fast interpreter
   don't have the same memory operation interface.
   Old interpreter framework does one access to coprocessor per data, and
   handles already data write, as well as address computation,
   which is not the case for Fast interpreter. Therefore, implementation
   of vfp instructions in old interpreter and fast interpreter are separate. */

/* ----------- STC ------------ */
#define VFP_STC_IMPL
#include "vfpinstr.c"
#undef VFP_STC_IMPL


/* ----------- LDC ------------ */
#define VFP_LDC_IMPL
#include "vfpinstr.c"
#undef VFP_LDC_IMPL


/* ----------- CDP ------------ */
#define VFP_CDP_IMPL
#include "vfpinstr.c"
#undef VFP_CDP_IMPL

/* Miscellaneous functions */
int32_t vfp_get_float(arm_core_t* state, unsigned int reg)
{
    DEBUG("VFP get float: s%d=[%08x]\n", reg, state->ExtReg[reg]);
    return state->ExtReg[reg];
}

void vfp_put_float(arm_core_t* state, int32_t val, unsigned int reg)
{
    DEBUG("VFP put float: s%d <= [%08x]\n", reg, val);
    state->ExtReg[reg] = val;
}

uint64_t vfp_get_double(arm_core_t* state, unsigned int reg)
{
    uint64_t result;
    result = ((uint64_t) state->ExtReg[reg*2+1])<<32 | state->ExtReg[reg*2];
    DEBUG("VFP get double: s[%d-%d]=[%016llx]\n", reg*2+1, reg*2, result);
    return result;
}

void vfp_put_double(arm_core_t* state, uint64_t val, unsigned int reg)
{
    DEBUG("VFP put double: s[%d-%d] <= [%08x-%08x]\n", reg*2+1, reg*2, (uint32_t) (val>>32), (uint32_t) (val & 0xffffffff));
    state->ExtReg[reg*2] = (uint32_t) (val & 0xffffffff);
    state->ExtReg[reg*2+1] = (uint32_t) (val>>32);
}



/*
 * Process bitmask of exception conditions. (from vfpmodule.c)
 */
void vfp_raise_exceptions(ARMul_State* state, u32 exceptions, u32 inst, u32 fpscr)
{
    int si_code = 0;

    vfpdebug("VFP: raising exceptions %08x\n", exceptions);

    if (exceptions == VFP_EXCEPTION_ERROR) {
        DEBUG("unhandled bounce %x\n", inst);
        exit(-1);
        return;
    }

    /*
     * If any of the status flags are set, update the FPSCR.
     * Comparison instructions always return at least one of
     * these flags set.
     */
    if (exceptions & (FPSCR_N|FPSCR_Z|FPSCR_C|FPSCR_V))
        fpscr &= ~(FPSCR_N|FPSCR_Z|FPSCR_C|FPSCR_V);

    fpscr |= exceptions;

    state->VFP[VFP_OFFSET(VFP_FPSCR)] = fpscr;
}
