/*****************************************************************
 * Description 
 * Email huxiaoheigame@gmail.com
 * Created on 2021/08/07
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/config.h"

#include <yaml-cpp/yaml.h>

#include "../src/log.h"

tigerkin::ConfigVar<int>::ptr intCfg = tigerkin::Config::Lookup("test.int", (int)8080, "int");
tigerkin::ConfigVar<float>::ptr floatCfg = tigerkin::Config::Lookup("test.float", (float)10.2f, "float");
tigerkin::ConfigVar<std::string>::ptr stringCfg = tigerkin::Config::Lookup("test.string", (std::string) "Hello", "string");
tigerkin::ConfigVar<std::vector<std::string>>::ptr vectorCfg = tigerkin::Config::Lookup("test.vector", (std::vector<std::string>){"Hello World"}, "vector");
tigerkin::ConfigVar<std::list<std::string>>::ptr listCfg = tigerkin::Config::Lookup("test.list", (std::list<std::string>){"Hello World"}, "list");
tigerkin::ConfigVar<std::set<int>>::ptr setCfg = tigerkin::Config::Lookup("test.set", (std::set<int>){1, 2, 3}, "set");
tigerkin::ConfigVar<std::unordered_set<int>>::ptr unorderedSetCfg = tigerkin::Config::Lookup("test.unordered_set", (std::unordered_set<int>){1, 2, 3}, "unordered_set");
tigerkin::ConfigVar<std::map<std::string, std::string>>::ptr mapCfg = tigerkin::Config::Lookup("test.map", (std::map<std::string, std::string>){{"key1", "Hello"}}, "map");
tigerkin::ConfigVar<std::unordered_map<std::string, std::string>>::ptr unorderedMapCfg = tigerkin::Config::Lookup("test.unordered_map", (std::unordered_map<std::string, std::string>){}, "unordered_map");

void print_yaml(YAML::Node node, int level) {
    if (node.IsScalar()) {
        TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << std::string(level * 4, ' ')
                                               << node.Scalar() << "~~";
    } else if (node.IsNull()) {
        TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << std::string(level * 4, ' ')
                                               << " NULL";
    } else if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << std::string(level * 4, ' ')
                                                   << it->first << ":" << it->second.Type();
            print_yaml(it->second, level + 1);
        }
    } else if (node.IsSequence()) {
        for (size_t i = 0; i < node.size(); ++i) {
            TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << std::string(level * 4, ' ')
                                                   << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    } else {
        TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << "Error:" << node.Type();
    }
}

void test_yaml() {
    YAML::Node rootNode = YAML::LoadFile("/home/liuhu/tigerkin/tests/test_conf.yml");
    print_yaml(rootNode, 0);
}

void test_config() {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << "int before:" << intCfg->getValue();
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << "float before:" << floatCfg->getValue();
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << "string before:" << stringCfg->getValue();
#define XX(name, prefix, value)                             \
    {                                                       \
        std::stringstream ss;                               \
        ss.str("");                                         \
        ss << #name << " " << #prefix << ": ";              \
        for (auto it : value) {                             \
            ss << it;                                       \
        }                                                   \
        TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << ss.str(); \
    }

#define XX_M(name, prefix, value)                           \
    {                                                       \
        std::stringstream ss;                               \
        ss.str("");                                         \
        ss << #name << " " << #prefix << ":";               \
        for (auto it : value) {                             \
            ss << it.first << ":" << it.second;             \
        }                                                   \
        TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << ss.str(); \
    }

    XX(vector, before, vectorCfg->getValue());
    XX(list, before, listCfg->getValue());
    XX(set, before, setCfg->getValue());
    XX(unordered_set, before, unorderedSetCfg->getValue());
    XX_M(map, before, mapCfg->getValue());
    XX_M(unordered_map, before, unorderedMapCfg->getValue());

    YAML::Node root = YAML::LoadFile("/home/liuhu/tigerkin/tests/test_conf.yml");
    tigerkin::Config::LoadFromYaml(root, "test");

    TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << "int before:" << intCfg->getValue();
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << "float before:" << floatCfg->getValue();
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << "string before:" << stringCfg->getValue();
    XX(vector, after, vectorCfg->getValue());
    XX(list, after, listCfg->getValue());
    XX(set, after, setCfg->getValue());
    XX(unordered_set, after, unorderedSetCfg->getValue());
    XX_M(map, after, mapCfg->getValue());
    XX_M(unordered_map, before, unorderedMapCfg->getValue());
#undef XX
#undef XX_M
}

class Persion {
   public:
    std::string name = "default";
    int age = 0;
    int sex = 0;

    std::string to_string() {
        std::stringstream ss;
        ss << "[Persion name:" << name
           << " age:" << age
           << " sex:" << sex
           << " ]";
        return ss.str();
    }
};

namespace tigerkin {

template <>
class LexicalCast<std::string, Persion> {
   public:
    Persion operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        Persion p;
        p.name = node["name"].as<std::string>();
        p.age = node["age"].as<int>();
        p.sex = node["sex"].as<int>();
        return p;
    }
};

template <>
class LexicalCast<Persion, std::string> {
   public:
    std::string operator()(const Persion &p) {
        YAML::Node node;
        node["name"] = p.name;
        node["age"] = p.age;
        node["sex"] = p.sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
}  // namespace tigerkin

void test_class() {
    tigerkin::ConfigVar<std::vector<Persion>>::ptr persionVerCfg = tigerkin::Config::Lookup("test.persion_vector", (std::vector<Persion>){Persion()}, "persion");

#define XX(name, prefix, value)                                                               \
    {                                                                                         \
        std::stringstream ss;                                                                 \
        for (auto it : value) {                                                               \
            ss << it.to_string();                                                             \
        }                                                                                     \
        TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << #name << " " << #prefix << " " << ss.str(); \
    }

    XX(persionVector, before, persionVerCfg->getValue());
    YAML::Node root = YAML::LoadFile("/home/liuhu/tigerkin/tests/test_conf.yml");
    tigerkin::Config::LoadFromYaml(root, "test");
    XX(persionVector, after, persionVerCfg->getValue());
#undef XX
}

int main(int argc, char **argv) {
    std::cout << "config test start" << std::endl;
    test_yaml();
    test_config();
    test_class();
    std::cout << "config test end" << std::endl;
    return 0;
}
