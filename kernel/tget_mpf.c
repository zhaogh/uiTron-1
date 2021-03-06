/*
 * Copyright (C) 2007,2008,2009,2010
 * 256TECH Co., Ltd.
 * Masahiro Sakamoto (m-sakamoto@users.sourceforge.net)
 *
 * This file is part of URIBO.
 *
 * URIBO is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * URIBO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with URIBO; see the file COPYING and COPYING.LESSER.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#include "kernel.h"

/*
 * get_mpf/pget_mpf/tget_mpf system call
 */
ER tget_mpf(ID id, VP *p_blf, TMO tmout)
{
    UINT psw;
    ER r = E_OK;
    T_MPF *mpf;
    
#ifndef _KERNEL_NO_STATIC_ERR
    {
        UB s = _kernel_sts;
        /*
         * check context
         */
        if (tmout == TMO_POL) {
            if (_KERNEL_CHK_LOC(s))
                _KERNEL_RET(E_CTX);
        } else {
            if (_KERNEL_CHK_WAI(s, tmout))
                _KERNEL_RET(E_CTX);
        }
        /*
         * check par
         */
        if (!id || _kernel_mpfid_max < (UINT)id)
            _KERNEL_RET(E_ID);
        if (!p_blf)
            _KERNEL_RET(E_PAR);
    }
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get momory block
     */
    if (!(mpf = _kernel_mpf[id - 1]))
        _KERNEL_END(E_NOEXS);
    {
        if (mpf->cnt) {
            mpf->cnt--;
            *p_blf = (VP)_kernel_deq_msg(&mpf->free, 1);
        }
        /*
         * get-memory-block-waiting
         */
        else {
            T_TCB *tcb;
            if (tmout == TMO_POL)
                _KERNEL_END_NOLOG(E_TMOUT);
            tcb = (T_TCB *)_kernel_cur;
            tcb->wai = TTW_MPF;
            tcb->wid = id;
            _KERNEL_SET_REGPAR_DAT(p_blf);
            _KERNEL_SET_REGPAR_FLG();
            r = _kernel_wai(tcb, tmout, &mpf->que[0],
                            mpf->cmpf->mpfatr & TA_TPRI); /* dispatch */
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_TGET_MPF();
    return r;
}

/* eof */
