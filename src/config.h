/*****************************************************************
 * Description config
 * Email huxiaoheigame@gmail.com
 * Created on 2021/08/04
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_CONFIG_H__
#define __TIGERKIN_CONFIG_H__

#include <yaml-cpp/yaml.h>

#include <boost/lexical_cast.hpp>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "log.h"
#include "thread.h"

namespace tigerkin {

template <class FromTyte, class ToType>
class LexicalCast {
   public:
    ToType operator()(const FromTyte &v) {
        return boost::lexical_cast<ToType>(v);
    }
};

template <class T>
class LexicalCast<std::string, std::vector<T>> {
   public:
    std::vector<T> operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        std::vector<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::vector<T>, std::string> {
   public:
    std::string operator()(const vector<T> &v) {
        YAML::Node node;
        for (auto &it : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <class T>
class LexicalCast<std::string, std::list<T>> {
   public:
    std::list<T> operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        std::list<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::list<T>, std::string> {
   public:
    std::string operator()(const list<T> &v) {
        YAML::Node node;
        for (auto &it : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <class T>
class LexicalCast<std::string, std::set<T>> {
   public:
    std::set<T> operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> res;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            res.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return res;
    }
};

template <class T>
class LexicalCast<std::set<T>, std::string> {
   public:
    std::string operator()(const std::set<T> &v) {
        YAML::Node node;
        for (auto &it : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <class T>
class LexicalCast<std::string, std::unordered_set<T>> {
   public:
    std::unordered_set<T> operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> res;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            res.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return res;
    }
};

template <class T>
class LexicalCast<std::unordered_set<T>, std::string> {
   public:
    std::string operator()(const std::unordered_set<T> &v) {
        YAML::Node node;
        for (auto &it : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <class T>
class LexicalCast<std::string, std::map<std::string, T>> {
   public:
    std::map<std::string, T> operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> res;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            res.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return res;
    }
};

template <class T>
class LexicalCast<std::map<std::string, T>, std::string> {
   public:
    std::string operator()(const std::map<std::string, T> &v) {
        YAML::Node node;
        for (auto &it : v) {
            node[it.first] = YAML::Load(LexicalCast<T, std::string>()(it.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
   public:
    std::unordered_map<std::string, T> operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> res;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            res.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return res;
    }
};

template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
   public:
    std::string operator()(const std::unordered_map<std::string, T> &v) {
        YAML::Node node;
        for (auto &it : v) {
            node[it.first] = YAML::Load(LexicalCast<T, std::string>()(it.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

class ConfigVarBase {
   public:
    typedef std::shared_ptr<ConfigVarBase> ptr;

    ConfigVarBase(const std::string name, const std::string &description = "")
        : m_name(name), m_description(description) {}
    virtual ~ConfigVarBase() {}

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string &val) = 0;

    const std::string &getName() { return m_name; }
    const std::string &getDescription() { return m_description; }

   protected:
    std::string m_name;
    std::string m_description;
};

template <class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
   public:
    typedef std::shared_ptr<ConfigVar> ptr;

    ConfigVar(const std::string &name, const T &defaultValue, const std::string &description = "")
        : ConfigVarBase(name, description), m_val(defaultValue) {}

    std::string toString() override {
        try {
            return ToStr()(getValue());
        } catch (const std::exception &e) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_ROOT()) << "ConfigVal::toString exception:" << e.what() << " convert: " << typeid(m_val).name() << " to string";
        }
        return "";
    }

    bool fromString(const std::string &val) override {
        try {
            setValue(FromStr()(val));
            return true;
        } catch (const std::exception &e) {            
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_ROOT()) << "ConfigVal::fromString expection:" << e.what() << " convert: " << typeid(val).name() << " from string";
        }
        return false;
    }

    const T getValue() { 
        ReadWriteMutex::ReadMutex lock(m_mutex);
        return m_val;
    }

    void setValue(const T &v) { 
        ReadWriteMutex::WriteMutex lock(m_mutex);
        m_val = v;
    }

   private:
    ReadWriteMutex m_mutex;
    T m_val;
};

class Config {
   public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string &name, const T &defaultValue, const std::string &description = "") {
        auto tmp = Lookup<T>(name);
        if (tmp) {
            TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << "lookup name: " << name << " exists";
            return tmp;
        }
        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._1234567890") != std::string::npos) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_ROOT()) << "lookup name invalid " << name;
            throw std::invalid_argument(name);
        }
        ReadWriteMutex::WriteMutex lock(GetMutex());
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, defaultValue, description));
        GetDatas()[name] = v;
        return v;
    }

    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string &name) {
        ReadWriteMutex::ReadMutex lock(GetMutex());
        auto it = GetDatas().find(name);
        if (it == GetDatas().end())
            return nullptr;
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static ConfigVarBase::ptr LookupBase(const std::string &name);
    static void LoadFromYaml(const YAML::Node &root, const std::string &name);

   private:
    static ConfigVarMap &GetDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }
    static ReadWriteMutex &GetMutex() {
        static ReadWriteMutex s_mutex;
        return s_mutex;
    }
    
};

}  // namespace tigerkin

#endif  // !__TIGERKIN__CONFIG_H__