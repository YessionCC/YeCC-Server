#pragma once

#include<memory>
#include<sstream>
#include<functional>
#include<string>
#include<exception>
#include<algorithm>
#include<map>
#include<list>
#include<set>
#include<unordered_map>
#include<unordered_set>
#include"log.hpp"
#include"boost/lexical_cast.hpp"
#include"yaml-cpp/yaml.h"
#include"thread.hpp"

namespace yecc {

  class ConfigVarBase {
    public:
      typedef std::shared_ptr<ConfigVarBase> ptr;
      ConfigVarBase(const std::string& name, const std::string& desp="")
        :m_name(name), m_description(desp){
          std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
      }
      virtual ~ConfigVarBase(){}

      const std::string& getName() const { return m_name; }
      const std::string& getDesp() const { return m_description; }
      
      virtual std::string toString() = 0;
      virtual bool fromString(const std::string& val) = 0;

      virtual std::string getTypeName() const = 0;
    protected:
      std::string m_name;
      std::string m_description;
      
  };

  template<class F, class T>
  class LexicalCast {
    public:
      T operator()(const F& v) {
        return boost::lexical_cast<T>(v);
      } 
  };

  template<class T>
  class LexicalCast<std::string, std::vector<T> > {
    //template partial specilization
    public:
      std::vector<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> ret;
        for(size_t i = 0; i<node.size(); i++) {
          std::stringstream ss; ss<<node[i];
          ret.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return ret;
      } 
  };

  template<class T>
  class LexicalCast<std::vector<T>, std::string> {
    //template partial specilization
    public:
      std::string operator()(const std::vector<T>& v) {
        YAML::Node node;
        for(auto d: v) {
          node.push_back(YAML::Node(LexicalCast<T, std::string>()(d)));
        }
        std::stringstream ss; ss<<node;
        return ss.str();
      } 
  };

  template<class T>
  class LexicalCast<std::string, std::list<T> > {
    //template partial specilization
    public:
      std::list<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::list<T> ret;
        for(size_t i = 0; i<node.size(); i++) {
          std::stringstream ss; ss<<node[i];
          ret.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return ret;
      } 
  };

  template<class T>
  class LexicalCast<std::list<T>, std::string> {
    //template partial specilization
    public:
      std::string operator()(const std::list<T>& v) {
        YAML::Node node;
        for(auto d: v) {
          node.push_back(YAML::Node(LexicalCast<T, std::string>()(d)));
        }
        std::stringstream ss; ss<<node;
        return ss.str();
      } 
  };

  template<class T>
  class LexicalCast<std::string, std::set<T> > {
    //template partial specilization
    public:
      std::set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> ret;
        for(size_t i = 0; i<node.size(); i++) {
          std::stringstream ss; ss<<node[i];
          ret.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return ret;
      } 
  };

  template<class T>
  class LexicalCast<std::set<T>, std::string> {
    //template partial specilization
    public:
      std::string operator()(const std::set<T>& v) {
        YAML::Node node;
        for(auto d: v) {
          node.push_back(YAML::Node(LexicalCast<T, std::string>()(d)));
        }
        std::stringstream ss; ss<<node;
        return ss.str();
      } 
  };

  template<class T>
  class LexicalCast<std::string, std::unordered_set<T> > {
    //template partial specilization
    public:
      std::unordered_set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> ret;
        for(size_t i = 0; i<node.size(); i++) {
          std::stringstream ss; ss<<node[i];
          ret.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return ret;
      } 
  };

  template<class T>
  class LexicalCast<std::unordered_set<T>, std::string> {
    //template partial specilization
    public:
      std::string operator()(const std::unordered_set<T>& v) {
        YAML::Node node;
        for(auto d: v) {
          node.push_back(YAML::Node(LexicalCast<T, std::string>()(d)));
        }
        std::stringstream ss; ss<<node;
        return ss.str();
      } 
  };

  template<class T>
  class LexicalCast<std::string, std::map<std::string, T> > {
    //template partial specilization
    public:
      std::map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> ret;
        for(auto p: node) {
          std::stringstream ss; ss<<p.second;
          ret[p.first.Scalar()]=LexicalCast<std::string, T>()(ss.str());
        }
        return ret;
      } 
  };

  template<class T>
  class LexicalCast<std::map<std::string, T>, std::string> {
    //template partial specilization
    public:
      std::string operator()(const std::map<std::string, T>& v) {
        YAML::Node node;
        for(auto d: v) {
          node[d.first] = YAML::Node(LexicalCast<T, std::string>()(d.second));
        }
        std::stringstream ss; ss<<node;
        return ss.str();
      } 
  };
  
  template<class T>
  class LexicalCast<std::string, std::unordered_map<std::string, T> > {
    //template partial specilization
    public:
      std::unordered_map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> ret;
        for(auto p: node) {
          std::stringstream ss; ss<<p.second;
          ret[p.first.Scalar()]=LexicalCast<std::string, T>()(ss.str());
        }
        return ret;
      } 
  };

  template<class T>
  class LexicalCast<std::unordered_map<std::string, T>, std::string> {
    //template partial specilization
    public:
      std::string operator()(const std::unordered_map<std::string, T>& v) {
        YAML::Node node;
        for(auto d: v) {
          node[d.first] = YAML::Node(LexicalCast<T, std::string>()(d.second));
        }
        std::stringstream ss; ss<<node;
        return ss.str();
      } 
  };

  //FromStr T operator()(const std::string&)
  //ToStr std::string operator()(const T&)
  template<
    class T, 
    class FromStr = LexicalCast<std::string, T>, //set default template
    class ToStr = LexicalCast<T, std::string>
  >
  class ConfigVar : public ConfigVarBase { //Convert base data type
    public:
      typedef RWMutex RWMutexType;
      typedef std::shared_ptr<ConfigVar> ptr;
      typedef std::function<void(const T& old_val, const T& new_val)> on_change_cb;
      ConfigVar(
        const std::string& name, 
        const T& default_val,
        const std::string& desp = ""
      ) :ConfigVarBase(name, desp), m_val(default_val) {}

      std::string toString() override {
        try {
          RWMutexType::ReadLock lock(m_mutex);
          return ToStr()(m_val); //boost::lexical_cast<std::string>(m_val);
        } catch(std::exception& e) {
          YECC_LOG_ERROR(YECC_ROOT_LOG) 
            << "ConfigVar::toString exception"
            << e.what() << " convert: " 
            << typeid(m_val).name() << "to string";
        }
        return "";
      }
      bool fromString(const std::string& val) override {
        try {
          //m_val = boost::lexical_cast<T>(val);
          setValue(FromStr()(val));
          return true;
        } catch(std::exception& e) {
          YECC_LOG_ERROR(YECC_ROOT_LOG) 
            << "ConfigVar::fromString exception"
            << e.what() << " convert: string to" 
            << typeid(m_val).name();
        }
        return false;
      }

      const T getValue() { 
        RWMutexType::ReadLock lock(m_mutex);
        return m_val; 
      }
      void setValue(const T& v) { 
        {
          RWMutexType::ReadLock lock(m_mutex);
          if(m_val == v) return;
          for(auto& p: m_cbs) {
            p.second(m_val, v);
          }
        }
        RWMutexType::WriteLock lock(m_mutex);
        m_val = v; 
      } 
      std::string getTypeName() const override { return typeid(T).name(); }

      uint64_t addListener(on_change_cb cb) {//return key
        static uint64_t key = 0;
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs[key] = cb;
        return key++;
      }
      void delListener(uint64_t key) {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.erase(key);
      }
      void clearListener() {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.clear();
      }
      on_change_cb getListener(uint64_t key) {
        RWMutexType::ReadLock lock(m_mutex);
        auto it = m_cbs.find(key);
        if(it == m_cbs.end()) return nullptr;
        else return it->second;
      }
    private:
      RWMutexType m_mutex;
      T m_val;

      std::map<uint64_t, on_change_cb> m_cbs;
  };

  class Config {
    public:
      typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;
      typedef RWMutex RWMutexType;

      template<class T>
      static typename ConfigVar<T>::ptr Lookup( //create
        const std::string& name, 
        const T& default_val,
        const std::string& desp = "") {
          RWMutexType::WriteLock lock(GetMutex());
          auto& data = GetDatas();
          auto it = data.find(name);
          if(it != data.end()) {
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T> > (it->second);
            if(tmp) {
              YECC_LOG_INFO(YECC_ROOT_LOG)<<"Lookup name = "<<name<<" exists";
              return tmp;
            }
            else {
              YECC_LOG_ERROR(YECC_ROOT_LOG)<<"Lookup name = "<<name
                <<" exists but type was "<<it->second->getTypeName()
                <<" not: "<<typeid(T).name()<<", it's value is "
                <<it->second->toString();
                return nullptr;
            }
          }
          if(name.find_first_not_of(
            "abcdefghijklmnopqrstuvwxyz._0123456789"
            ) != std::string::npos) {
              YECC_LOG_ERROR(YECC_ROOT_LOG)<<"Lookup name invalid "<<name;
              throw std::invalid_argument(name);
          }
          typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_val, desp));
          data[name] = v;
          return v;
      }

      template<class T>
      static typename ConfigVar<T>::ptr Lookup(const std::string& name) { //find
        RWMutexType::ReadLock lock(GetMutex());
        auto& data = GetDatas();
        auto it = data.find(name);
        if(it == data.end()) return nullptr;
        else return std::dynamic_pointer_cast<ConfigVar<T> > (data[name]);
        //Note that parent class to child class must dynamic_pointer_cast
      }

      static ConfigVarBase::ptr LookupBase(const std::string& name) {
        RWMutexType::ReadLock lock(GetMutex());
        auto& data = GetDatas();
        auto it = data.find(name);
        if(it == data.end()) return nullptr;
        else return data[name];
      }

      static void LoadFromYaml(const YAML::Node& root);
      static void Visit(std::function<void(ConfigVarBase::ptr)> cb);

    private:
      static ConfigVarMap& GetDatas() {
        //to avoid static member init sequence causing vars not init
        static ConfigVarMap s_data;
        return s_data;
      }
      static RWMutexType& GetMutex() {
        static RWMutexType m_mutex;
        return m_mutex;
      }
  };

}