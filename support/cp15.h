/*
 * cp15.h
 *
 *  Created on: Apr. 14, 2021
 *      Author: renee.cousins
 */

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
