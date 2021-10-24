/*****************************************************************
 * Description Hook
 * Email huxiaoheigame@gmail.com
 * Created on 2021/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_HOOK_H__
#define __TIGERKIN_HOOK_H__

namespace tigerkin {

void setEnableHook(bool enable);
bool isEnableHook();

void blockSleep(unsigned int s);
void blockUsleep(unsigned int s);

void nonblockSleep(unsigned int s);
void nonblockUsleep(unsigned int us);

}  // namespace tigerkin

#endif
