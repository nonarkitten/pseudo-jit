/******************************************************************************
**
** cp15.h - CP15 coprocesser configuration APIs
**
********************************************************************************
**
** Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
**
**
**  Redistribution and use in source and binary forms, with or without
**  modification, are permitted provided that the following conditions
**  are met:
**
**    Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
**
**    Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the
**    distribution.
**
**    Neither the name of Texas Instruments Incorporated nor the names of
**    its contributors may be used to endorse or promote products derived
**    from this software without specific prior written permission.
**
**  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
**  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
**  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
**  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
**  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
**  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
**  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
**  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
**  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
**  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
**  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
********************************************************************************/

#include <stdint.h>

#ifndef CP15_H_
#define CP15_H_

extern void CP15AuxControlFeatureEnable(uint32_t enFlag);
extern void CP15AuxControlFeatureDisable(uint32_t disFlag);
extern void CP15DCacheCleanBuff(uint32_t bufPtr, uint32_t size);
extern void CP15DCacheCleanFlushBuff(uint32_t bufPtr, uint32_t size);
extern void CP15DCacheFlushBuff(uint32_t bufPtr, uint32_t size);
extern void CP15ICacheFlushBuff(uint32_t bufPtr, uint32_t size);
extern void CP15ICacheDisable(void);
extern void CP15DCacheDisable(void);
extern void CP15ICacheEnable(void);
extern void CP15DCacheEnable(void);
extern void CP15DCacheCleanFlush(void);
extern void CP15DCacheClean(void);
extern void CP15DCacheFlush(void);
extern void CP15ICacheFlush(void);
extern void CP15Ttb0Set(uint32_t ttb);
extern void CP15TlbInvalidate(void);
extern void CP15MMUDisable(void);
extern void CP15MMUEnable(void);
extern void CP15VectorBaseAddrSet(uint32_t addr);
extern void CP15BranchPredictorInvalidate(void);
extern void CP15BranchPredictionEnable(void);
extern void CP15BranchPredictionDisable(void);
extern void CP15DomainAccessClientSet(void);
extern void CP15ControlFeatureDisable(uint32_t features);
extern void CP15ControlFeatureEnable(uint32_t features);
extern void CP15TtbCtlTtb0Config(void);
extern uint32_t CP15MainIdPrimPartNumGet(void);


#endif /* CP15_H_ */