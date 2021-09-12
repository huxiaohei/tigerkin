/*****************************************************************
 * Description config
 * Email huxiaoheigame@gmail.com
 * Created on 2021/08/04
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "config.h"

namespace tigerkin {

bool isValidName(const std::string &name) {
    if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._1234567890") == std::string::npos)
        return true;
    return false;
}

static void listAllMember(const std::string &prefix,
                          const YAML::Node &node,
                          std::list<std::pair<std::string, const YAML::Node>> &output) {
    if (!isValidName(prefix) || prefix.empty()) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
        return;
    }
    output.push_back(std::make_pair(prefix, node));
    if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            listAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(), it->second, output);
        }
    }
}

ConfigVarBase::ptr Config::LookupBase(const std::string &name) {
    ReadWriteMutex::ReadMutex lock(GetMutex());
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
}

void Config::LoadFromYaml(const YAML::Node &root, const std::string &name) {
    std::list<std::pair<std::string, const YAML::Node>> allNodes;
    listAllMember(name, root, allNodes);
    for (auto &i : allNodes) {
        std::string key = i.first;
        if (key.empty())
            continue;
        ConfigVarBase::ptr var = LookupBase(key);
        if (var) {
            if (i.second.IsScalar()) {
                var->fromString(i.second.Scalar());
            } else {
                std::stringstream ss;
                ss << i.second;
                var->fromString(ss.str());
            }
        }
    }
}

}  // namespace tigerkin
