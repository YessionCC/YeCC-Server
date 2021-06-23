#include"log.hpp"
#include"config.hpp"
#include<iostream>
#include<functional>
#include<exception>

namespace yecc {

  LogEvent::LogEvent(
        std::shared_ptr<Logger> logger, LogLevel::Level level,
        const char* file, int32_t line, uint32_t elapse,
        uint32_t threadId, uint32_t fiberId, uint64_t time)
    :m_logger(logger), m_level(level), m_file(file), 
    m_line(line), m_elapse(elapse), m_threadId(threadId),
    m_fiberId(fiberId), m_time(time) {
  }

  void LogEvent::format(const char* fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
  }
  void LogEvent::format(const char* fmt, va_list al)  {
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if(len != -1) {
      m_ss<<std::string(buf, len);
      free(buf);
    }
  }

  LogEventWrap::~LogEventWrap(){
    m_logEvent->getLogger()->log(m_logEvent->getLevel(), m_logEvent);
  }

  Logger::Logger(const std::string& name)
    :m_name(name), m_level(LogLevel::DEBUG) {
      m_formatter.reset(new LogFormatter(
        "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
      ));
      /*
        default format of output
        func: reset
        just like = shared_ptr(new ...) 
      */
  }

  void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
      MutexType::Lock lock(m_mutex);
      auto self = shared_from_this();
      // this -> shared_ptr
      // must legacy from std::enable_shared_from_this<Logger>
      if(!m_appenders.empty()){
        for(LogAppender::ptr& app: m_appenders){
          app->log(self, level, event);
        }
      }
      else if(m_root){
        m_root->log(level, event);
      }
    }
  }

  void Logger::addAppender(LogAppender::ptr appender) {
    MutexType::Lock lock(m_mutex);
    if(!appender->getFormatter()) {
      appender->setFormatter(m_formatter);
      // if appender not set formatter
      // then use default(logger's appender)
    }
    m_appenders.push_back(appender);
  }
  void Logger::delAppender(LogAppender::ptr appender) {
    MutexType::Lock lock(m_mutex);
    m_appenders.remove(appender);
  }
  void Logger::clearAppender() {
    MutexType::Lock lock(m_mutex);
    m_appenders.clear();
  }

  void Logger::setFormatter(LogFormatter::ptr val){
    MutexType::Lock lock(m_mutex);
    m_formatter = val;
    for(auto& app: m_appenders) {
      if(!app->HasFormatter()) app->setFormatter(val);
    }
  }
  void Logger::setFormatter(std::string val){
    LogFormatter::ptr new_val(new LogFormatter(val));
    if(new_val->IsError()) {
      std::cout<<"Logger setFormatter name = "<<m_name
        <<" value = "<<val<<" invalid formatter"<<std::endl;
    }
    MutexType::Lock lock(m_mutex);
    for(auto& app: m_appenders) {
      if(!app->HasFormatter()) app->setFormatter(new_val);
    }
    m_formatter = new_val;
  }
  LogFormatter::ptr Logger::getFormatter(){
    return m_formatter;
  }

  void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::DEBUG, event);
  }
  void Logger::info(LogEvent::ptr event) {
    log(LogLevel::INFO, event);
  }
  void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::WARN, event);
  }
  void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::FATAL, event);
  }

  const char* LogLevel::ToString(Level level) { //static method remove 'static'
    switch(level) {
      case LogLevel::DEBUG: return "DEBUG";
      case LogLevel::INFO: return "INFO";
      case LogLevel::WARN: return "WARN";
      case LogLevel::ERROR: return "ERROR";
      case LogLevel::FATAL: return "FATAL";
      default: return "UNKNOW";
    }
  }

  LogLevel::Level LogLevel::FromString(std::string str){
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    if(str=="DEBUG") return LogLevel::DEBUG;
    else if(str=="INFO") return LogLevel::INFO;
    else if(str=="WARN") return LogLevel::WARN;
    else if(str=="ERROR") return LogLevel::ERROR;
    else if(str=="FATAL") return LogLevel::FATAL;
    else return LogLevel::UNKNOW;
  }

  LogFormatter::ptr LogAppender::getFormatter(){
    MutexType::Lock lock(m_mutex);
    return m_formatter;
  }

  //use formatter to set formatter will be considered as not has formatter
  void LogAppender::setFormatter(LogFormatter::ptr val){ 
    MutexType::Lock lock(m_mutex);
    m_formatter = val;
  }
  //use str to set formatter will be considered as has formatter
  void LogAppender::setFormatter(std::string val){
    LogFormatter::ptr new_val(new LogFormatter(val));
    if(new_val->IsError()) {
      std::cout<<"LogAppender setFormatter"
        <<" value = "<<val<<" invalid formatter"<<std::endl;
    }
    MutexType::Lock lock(m_mutex);
    m_formatter = new_val;
    has_formatter = true;
  }

  FileAppender::FileAppender(const std::string& name)
    :m_filename(name){
    m_type = 1;
  }

  void StdoutAppender::log(
    std::shared_ptr<Logger> ptr,
    LogLevel::Level level, 
    LogEvent::ptr event) {
    if(level >= m_level){
      MutexType::Lock lock(m_mutex);
      std::cout<<m_formatter->format(ptr, level, event);
    }
  }

  void FileAppender::log(
    std::shared_ptr<Logger> ptr,
    LogLevel::Level level, 
    LogEvent::ptr event) {
    if(level >= m_level) {
      uint64_t now = time(0);
      if(now != m_last_time) {//always keep file open(file may be deleted sometime)
        m_last_time = now;
        reopen();
      }
      MutexType::Lock lock(m_mutex);
      m_filestream<<m_formatter->format(ptr, level, event);
    }
  }

  bool FileAppender::reopen(){
    MutexType::Lock lock(m_mutex);
    if(m_filestream) m_filestream.close();
    m_filestream.open(m_filename, std::ios::app);//No justice
    return !!m_filestream; // !-> if failed; !!->if success
  }

  LogFormatter::LogFormatter(std::string pattern) 
    :m_pattern(pattern){
    init();
  }

  std::string LogFormatter::format(
    std::shared_ptr<Logger> ptr,
    LogLevel::Level level, 
    LogEvent::ptr event) {
    std::stringstream ss;
    /*
      stringstream legacy from isream and ostream and base_stringstream...
      also, like fstream/ifstream/ofstream legacy from isream and ostream
      stringstream is convenient for transforming number to string...
      and can use it like cout. 
    */
    for(auto& item: m_items) {
      item->format(ss, ptr, level, event);
    }
    return ss.str();
  }

  class MessageFormatItem: public LogFormatter::FormatItem {
  public:   
    MessageFormatItem(const std::string& str=""):FormatItem(str){} 
    void format(std::ostream& os, 
      std::shared_ptr<Logger> ptr,
      LogLevel::Level level, 
      LogEvent::ptr event) override {
      os<<event->getContent();//std::string
    }
  };
  class LevelFormatItem: public LogFormatter::FormatItem {
  public:
    LevelFormatItem(const std::string& str=""):FormatItem(str){} 
    void format(std::ostream& os, 
      std::shared_ptr<Logger> ptr,
      LogLevel::Level level, 
      LogEvent::ptr event) override {
      os<<LogLevel::ToString(level);//const char*
    }
  };
  class ElapseFormatItem: public LogFormatter::FormatItem {
  public:
    ElapseFormatItem(const std::string& str=""):FormatItem(str){} 
    void format(std::ostream& os, 
      std::shared_ptr<Logger> ptr,
      LogLevel::Level level, 
      LogEvent::ptr event) override {
      os<<event->getElapse();
    }
  };
  class StringFormatItem: public LogFormatter::FormatItem {
  public:
    StringFormatItem(const std::string& str)
      :FormatItem(str), m_string(str) {}
    void format(std::ostream& os, 
      std::shared_ptr<Logger> ptr,
      LogLevel::Level level, 
      LogEvent::ptr event) override {
      os<<m_string;
    }
  private:
    std::string m_string;
  };
  class LogNameFormatItem: public LogFormatter::FormatItem {
  public:
    LogNameFormatItem(const std::string& str=""):FormatItem(str){} 
    void format(std::ostream& os, 
      std::shared_ptr<Logger> ptr,
      LogLevel::Level level, 
      LogEvent::ptr event) override {
      os<<event->getLogger()->getName();
    }
  };
  class ThreadIdFormatItem: public LogFormatter::FormatItem {
  public:
    ThreadIdFormatItem(const std::string& str=""):FormatItem(str){} 
    void format(std::ostream& os, 
      std::shared_ptr<Logger> ptr,
      LogLevel::Level level, 
      LogEvent::ptr event) override {
      os<<event->getThreadId();
    }
  };
  class FiberIdFormatItem: public LogFormatter::FormatItem {
  public:
    FiberIdFormatItem(const std::string& str=""):FormatItem(str){} 
    void format(std::ostream& os, 
      std::shared_ptr<Logger> ptr,
      LogLevel::Level level, 
      LogEvent::ptr event) override {
      os<<event->getFiberId();
    }
  };
  class DateTimeFormatItem: public LogFormatter::FormatItem {
  public:
    DateTimeFormatItem(const std::string& format = "%Y:%m:%d %H:%M:%S")
      :FormatItem(format), m_format(format){}
    void format(std::ostream& os, 
      std::shared_ptr<Logger> ptr,
      LogLevel::Level level, 
      LogEvent::ptr event) override {
      struct tm tm;
      time_t time = event->getTime();
      localtime_r(&time, &tm);
      char buf[50];
      strftime(buf, sizeof(buf), m_format.c_str(), &tm);
      os<<buf;
    }
  private:
    std::string m_format;
  };
  class FilenameFormatItem: public LogFormatter::FormatItem {
  public:
    FilenameFormatItem(const std::string& str=""):FormatItem(str){} 
    void format(std::ostream& os, 
      std::shared_ptr<Logger> ptr,
      LogLevel::Level level, 
      LogEvent::ptr event) override {
      os<<event->getFile();
    }
  };
  class LineFormatItem: public LogFormatter::FormatItem {
  public:
    LineFormatItem(const std::string& str=""):FormatItem(str){} 
    void format(std::ostream& os, 
      std::shared_ptr<Logger> ptr,
      LogLevel::Level level, 
      LogEvent::ptr event) override {
      os<<event->getLine();
    }
  };
  class NewLineFormatItem: public LogFormatter::FormatItem {
  public:
    NewLineFormatItem(const std::string& str=""):FormatItem(str){} 
    void format(std::ostream& os, 
      std::shared_ptr<Logger> ptr,
      LogLevel::Level level, 
      LogEvent::ptr event) override {
      os<<std::endl;
    }
  };
  class TabFormatItem: public LogFormatter::FormatItem {
  public:
    TabFormatItem(const std::string& str=""):FormatItem(str){} 
    void format(std::ostream& os, 
      std::shared_ptr<Logger> ptr,
      LogLevel::Level level, 
      LogEvent::ptr event) override {
      os<<"\t";
    }
  };

  void LogFormatter::init() {
    std::vector<std::tuple<std::string, std::string, int>> parse_tab;
    //code_str, code_detail_str, if str
    //std::cout<<m_pattern<<std::endl;
    int len = m_pattern.size(), last_idx = 0;
    for(int i = 0; i<len; i++) {
      if(m_pattern[i] == '%') {
        if(last_idx != i) 
          parse_tab.push_back(std::make_tuple(
            m_pattern.substr(last_idx, i-last_idx), "", 0)
          );
        if(i+1 >= len) continue;
        last_idx = i+2;
        if(m_pattern[i+1]=='%') {
          parse_tab.push_back(std::make_tuple("%", "", 0));
          continue;
        }
        std::string code; code.push_back(m_pattern[i+1]);
        if(i+2 >= len || m_pattern[i+2]!='{') {
          parse_tab.push_back(std::make_tuple(code, "", 1));
          continue;
        }
        int idx = i+3;
        while(idx<len && m_pattern[idx]!='}') idx++;
        if(idx >= len) continue;
        parse_tab.push_back(std::make_tuple(
          code, m_pattern.substr(last_idx+1, idx-last_idx-1), 1)
        );
        last_idx = idx+1;
        i = idx;
      }
    }
    if(last_idx!=len) {
      parse_tab.push_back(std::make_tuple(
        m_pattern.substr(last_idx, len-last_idx), "", 0)
      );
    }

    static std::map<std::string, 
      std::function<FormatItem::ptr(const std::string& fmt)>
    > s_format_items = {
#define XX(str, C) \
      {#str, [](const std::string& fmt){ \
        return FormatItem::ptr(new C(fmt));}}
      XX(m, MessageFormatItem),
      XX(p, LevelFormatItem),
      XX(r, ElapseFormatItem),
      XX(c, LogNameFormatItem),
      XX(t, ThreadIdFormatItem),
      XX(n, NewLineFormatItem),
      XX(d, DateTimeFormatItem),
      XX(f, FilenameFormatItem),
      XX(l, LineFormatItem),
      XX(T, TabFormatItem),
      XX(F, FiberIdFormatItem)
#undef XX
    };

    for(auto tup: parse_tab) {
      // std::cout<<"{"<<std::get<0>(tup)<<"}{"
      //   <<std::get<1>(tup)<<"}{"<<std::get<2>(tup)<<"}"<<std::endl;
      if(std::get<2>(tup) == 0) 
        m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(tup))));
      else {
        std::string code = std::get<0>(tup);
        if(s_format_items.count(code) == 0) {
          m_items.push_back(FormatItem::ptr(new StringFormatItem("<error type>")));
          m_isError = true;
        }
        else 
          m_items.push_back(s_format_items[code](std::get<1>(tup)));
      }
    }
  }

  LoggerManager::LoggerManager() {
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutAppender));

    init();
  }
  Logger::ptr LoggerManager::getLogger(const std::string& name) {
    MutexType::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if(it != m_loggers.end()) return it->second;
    Logger::ptr nlogger(new Logger(name));
    nlogger->m_root = m_root;
    m_loggers[name] = nlogger;
    return nlogger;
  }

  struct LogAppenderDefine{
    int type = 0;
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::string file;

    bool operator==(const LogAppenderDefine& rhs) const {
      return type==rhs.type && level==rhs.level &&
        formatter==rhs.formatter && file==rhs.file;
    }
  };

  struct LogDefine{
    std::string name;
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;

    bool operator==(const LogDefine& rhs) const {
      return name==rhs.name && level==rhs.level &&
        formatter==rhs.formatter && appenders==rhs.appenders;
    }

    bool operator<(const LogDefine& rhs) const {
      return name<rhs.name;
    }//set use'<' to construct tree and judge equal
    // '==' won't use in set
  };

  template<>//template full specialization
  class LexicalCast<std::string, LogAppenderDefine > {
    public:
      LogAppenderDefine operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        LogAppenderDefine ret;
        if(!node["type"].IsDefined()) {
          throw std::invalid_argument("log conf: appender type name not define!");
        }
        std::string tp = node["type"].as<std::string>();
        if(tp == "StdoutLogAppender") ret.type = 2;
        else if(tp == "FileLogAppender") {
          ret.type = 1;
          if(!node["file"].IsDefined()) {
            throw std::invalid_argument(
              "log conf: fileAppender need file path!");
          }
          ret.file = node["file"].as<std::string>();
        }
        else {
          throw std::invalid_argument(
            "log conf: appender type name: "+tp+" invalid!");
        }
        if(node["level"].IsDefined())
          ret.level = LogLevel::FromString(node["level"].as<std::string>());
        if(node["formatter"].IsDefined())
          ret.formatter = node["formatter"].as<std::string>();
        return ret;
      } 
  };

  template<>
  class LexicalCast<LogAppenderDefine, std::string> {
    public:
      std::string operator()(const LogAppenderDefine& v) {
        YAML::Node node;
        if(v.type == 1) {
          node["type"] = "FileLogAppender";
          node["file"] = v.file;
        }
        else if(v.type == 2) node["type"] = "StdoutLogAppender";
        node["level"] = LogLevel::ToString(v.level);
        node["formatter"] = v.formatter;
        std::stringstream ss; ss<<node;
        return ss.str();
      } 
  };

  template<>//template full specialization
  class LexicalCast<std::string, LogDefine > {
    public:
      LogDefine operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        if(!node["name"].IsDefined()) {
          throw std::invalid_argument("log config error: no name!");
        }
        LogDefine ret;
        ret.name = node["name"].as<std::string>();
        if(node["level"].IsDefined())
          ret.level = LogLevel::FromString(node["level"].as<std::string>());
        if(node["formatter"].IsDefined())
          ret.formatter = node["formatter"].as<std::string>();
        std::stringstream ss; ss<<node["appender"];
        ret.appenders = 
          LexicalCast<std::string, std::vector<LogAppenderDefine> >()(ss.str());
        return ret;
      } 
  };

  template<>
  class LexicalCast<LogDefine, std::string> {
    public:
      std::string operator()(const LogDefine& v) {
        YAML::Node node;
        node["name"] = v.name;
        node["level"] = LogLevel::ToString(v.level);
        node["formatter"] = v.formatter;
        node["appender"] = 
          LexicalCast<std::vector<LogAppenderDefine>, std::string>()(v.appenders);
        std::stringstream ss; ss<<node;
        return ss.str();
      } 
  };


  std::string Logger::ToYamlStr() {
    LogDefine def;
    def.name = m_name;
    def.level = m_level;
    MutexType::Lock lock(m_mutex);
    def.formatter = m_formatter->getPattern();
    for(auto& app: m_appenders) {
      LogAppenderDefine apdef;
      apdef.type = app->getType();
      apdef.formatter = app->getFormatter()->getPattern();
      apdef.level = app->getLevel();
      if(apdef.type == 1) apdef.file = app->getFileName();
      def.appenders.push_back(apdef);
    }
    return LexicalCast<LogDefine, std::string>()(def);
  }

  ConfigVar<std::set<LogDefine> >::ptr logs_define = 
    Config::Lookup("logs", std::set<LogDefine>(), "logs config");

  struct LogIniter {
    LogIniter(){
      logs_define->addListener(0xFF1234, [](
        const std::set<LogDefine>& old,
        const std::set<LogDefine>& _new){
          for(auto& v: _new) {
            auto it = old.find(v);
            Logger::ptr logger;
            if(it==old.end()) {//new 
              YECC_LOG_INFO(YECC_ROOT_LOG)<<"logger conf changed: "
                <<"new logger added, name: "<<v.name;
              logger = YECC_LOG_NAME(v.name);
            }
            else {
              if(!(v==*it)) {// modify
                YECC_LOG_INFO(YECC_ROOT_LOG)<<"logger conf changed: "
                  <<"logger conf modified, name: "<<v.name;
                logger = YECC_LOG_NAME(v.name);
              }
            }
            if(!logger) return;// no change
            logger->setLevel(v.level);
            if(!v.formatter.empty()) {
              logger->setFormatter(v.formatter);
            }
            logger->clearAppender();
            for(auto& a: v.appenders) {
              LogAppender::ptr np;
              if(a.type == 1) {
                np.reset(new FileAppender(a.file));
              }
              else if(a.type == 2) {
                np.reset(new StdoutAppender);
              }
              if(a.level != LogLevel::UNKNOW)
                np->setLevel(a.level);
              if(!a.formatter.empty())
                np->setFormatter(a.formatter);
              logger->addAppender(np);
            }
          }
          for(auto& v: old) {
            auto it = _new.find(v);
            if(it==_new.end()) {// delete
              YECC_LOG_INFO(YECC_ROOT_LOG)<<"logger conf changed: "
                  <<"logger deleted, name: "<<v.name;
              auto logger = YECC_LOG_NAME(v.name);
              logger->setLevel((LogLevel::Level)100);
              logger->clearAppender();
            }
          }
        });
    }
  };
  static LogIniter __log_init;// static var to init before main()

  void LoggerManager::init() {

  }

}