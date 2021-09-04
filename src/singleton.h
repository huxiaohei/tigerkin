/*****************************************************************
 * Description singleton
 * Email huxiaoheigame@gmail.com
 * Created on 2021/08/04
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_SINGLETON_H__
#define __TIGERKIN_SINGLETON_H__

#include <memory>

namespace tigerkin {

template <class T>
class Singleton {
   public:
    static T* GetInstance() {
        static T v;
        return &v;
    }
};

template <class T>
class SingletonPtr {
   public:
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    }
};

}  // namespace tigerkin

#endif  // !__TIGERKIN_SINGLETON_H__
