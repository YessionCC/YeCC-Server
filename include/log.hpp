#pragma once

#include<map>
#include<string>
#include<stdint.h>
#include<stdarg.h>
#include<memory>
#include<list>
#include<sstream>
#include<vector>
#include<fstream>
#include"singleton.hpp"
#include"util.hpp"
#include"thread.hpp"

#define YECC_LOG_LEVEL(logger, level) \
  if(logger->getLevel() <= level) \
    yecc::LogEventWrap(yecc::LogEvent::ptr(new yecc::LogEvent( \
      logger, level, \
      __FILE__, __LINE__, 0, yecc::GetThreadId(), \
      yecc::GetFiberId(), time(0), yecc::Thread::GetName()))).getSS()

#define YECC_LOG_DEBUG(logger) YECC_LOG_LEVEL(logger, yecc::LogLevel::DEBUG)
#define YECC_LOG_INFO(logger) YECC_LOG_LEVEL(logger, yecc::LogLevel::INFO)
#define YECC_LOG_WARN(logger) YECC_LOG_LEVEL(logger, yecc::LogLevel::WARN)
#define YECC_LOG_ERROR(logger) YECC_LOG_LEVEL(logger, yecc::LogLevel::ERROR)
#define YECC_LOG_FATAL(logger) YECC_LOG_LEVEL(logger, yecc::LogLevel::FATAL)

#define YECC_LOG_FMT_LEVEL(logger, level, fmt, ...) \
  if(logger->getLevel() <= level) \
    yecc::LogEventWrap(yecc::LogEvent::ptr(new yecc::LogEvent( \
      logger, level, \
      __FILE__, __LINE__, 0, yecc::GetThreadId(), \
      yecc::GetFiberId(), time(0), yecc::Thread::GetName()))).\
      getEvent()->format(fmt, __VA_ARGS__)

#define YECC_LOG_FMT_DEBUG(logger, fmt, ...) \
  YECC_LOG_FMT_LEVEL(logger, yecc::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define YECC_LOG_FMT_INFO(logger, fmt, ...) \
  YECC_LOG_FMT_LEVEL(logger, yecc::LogLevel::INFO, fmt, __VA_ARGS__)
#define YECC_LOG_FMT_WARN(logger, fmt, ...) \
  YECC_LOG_FMT_LEVEL(logger, yecc::LogLevel::WARN, fmt, __VA_ARGS__)
#define YECC_LOG_FMT_ERROR(logger, fmt, ...) \
  YECC_LOG_FMT_LEVEL(logger, yecc::LogLevel::ERROR, fmt, __VA_ARGS__)
#define YECC_LOG_FMT_FATAL(logger, fmt, ...) \
  YECC_LOG_FMT_LEVEL(logger, yecc::LogLevel::FATAL, fmt, __VA_ARGS__)

#define YECC_ROOT_LOG yecc::LoggerMgr::GetInstance()->getRoot()
#define YECC_LOG_NAME(name) yecc::LoggerMgr::GetInstance()->getLogger(name)
//get old or create new Logger with name

namespace yecc {

  class Logger;
  class LoggerManager;

  class LogLevel {
    public:
      enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
      };
    
      static const char* ToString(Level level);
      static LogLevel::Level FromString(std::string str);
  };

  class LogEvent : public std::enable_shared_from_this<LogEvent>{ // detailed content of logs
    public:
      typedef std::shared_ptr<LogEvent> ptr;
      LogEvent(
        std::shared_ptr<Logger> logger, LogLevel::Level level,
        const char* file, int32_t line, uint32_t elapse,
        uint32_t threadId, uint32_t fiberId, uint64_t time, 
        const std::string& thread_name);

      const char* getFile() const { return m_file; }
      int32_t getLine() const { return m_line; }
      uint32_t getElapse() const { return m_elapse; }
      uint32_t getThreadId() const { return m_threadId; }
      uint32_t getFiberId() const { return m_fiberId; }
      uint64_t getTime() const { return m_time; }
      const std::string& getThreadName() const { return m_tname; }
      const std::string getContent() const { return m_ss.str(); }
      std::stringstream& getSS() { return m_ss; }

      std::shared_ptr<Logger> getLogger() const {return m_logger;}
      LogLevel::Level getLevel() const {return m_level;}

      void format(const char* fmt, ...);
      void format(const char* fmt, va_list al);
    private:
      std::shared_ptr<Logger> m_logger;
      LogLevel::Level m_level;

      const char* m_file = nullptr; 
      int32_t m_line = 0;
      uint32_t m_elapse = 0; //millseconds from program start
      uint32_t m_threadId = 0;
      uint32_t m_fiberId = 0; // corontine ID
      uint64_t m_time;
      const std::string m_tname;//thread name
      std::stringstream m_ss;
  };

  class LogEventWrap {
    public:
      LogEventWrap(LogEvent::ptr logEvent):m_logEvent(logEvent){}
      ~LogEventWrap();
      std::stringstream& getSS() { return m_logEvent->getSS(); }
      LogEvent::ptr getEvent() { return m_logEvent; }
    private:
      LogEvent::ptr m_logEvent;
  };

  class LogFormatter { //format logevent
    public:
      typedef std::shared_ptr<LogFormatter> ptr;
      LogFormatter(std::string pattern);

      std::string format(
        std::shared_ptr<Logger> ptr,
        LogLevel::Level level, 
        LogEvent::ptr event
      );
    public:
      class FormatItem {
        public:
          typedef std::shared_ptr<FormatItem> ptr;
          FormatItem(const std::string& fmt = ""){}
          virtual ~FormatItem() {}
          virtual void format(std::ostream& os, 
            std::shared_ptr<Logger> ptr,
            LogLevel::Level level, 
            LogEvent::ptr event
          ) = 0;
      };

      void init();
      bool IsError(){return m_isError;}
      const std::string& getPattern() const {return m_pattern;}
    private:
      std::string m_pattern;
      std::vector<FormatItem::ptr> m_items;
      bool m_isError = false;
  };

  class LogAppender { // Log Output to where?
    public:
      typedef std::shared_ptr<LogAppender> ptr;
      typedef Mutex MutexType;
      LogAppender(LogLevel::Level level=LogLevel::DEBUG):m_level(level){}
      virtual ~LogAppender(){}

      void setLevel(LogLevel::Level level) {m_level=level;}
      LogLevel::Level getLevel() {return m_level;}
      virtual void log(
        std::shared_ptr<Logger> ptr,
        LogLevel::Level level, 
        LogEvent::ptr event
      ) = 0;
      void setFormatter(LogFormatter::ptr fomatter);
      void setFormatter(std::string fomatter);
      LogFormatter::ptr getFormatter();
      int getType() const {return m_type;}
      virtual const std::string getFileName() {return "";}//Only for fileapp
      bool HasFormatter() { return has_formatter; }
    protected:
      LogLevel::Level m_level;
      LogFormatter::ptr m_formatter;
      MutexType m_mutex;
      int m_type;
      bool has_formatter = false;//if set formatter, not use default?
  };

  class Logger: public std::enable_shared_from_this<Logger> {
    friend class LoggerManager;//friend class can visit private member
    public:
      typedef std::shared_ptr<Logger> ptr;
      typedef Mutex MutexType;
      Logger(const std::string& name = "root");

      void log(LogLevel::Level level, LogEvent::ptr event);

      void debug(LogEvent::ptr event);
      void info(LogEvent::ptr event);
      void warn(LogEvent::ptr event);
      void fatal(LogEvent::ptr event);

      void addAppender(LogAppender::ptr appender);
      void clearAppender();
      void delAppender(LogAppender::ptr appender);

      void setFormatter(LogFormatter::ptr val);
      void setFormatter(std::string val);
      LogFormatter::ptr getFormatter();

      void setLevel(LogLevel::Level level) {m_level = level;}
      LogLevel::Level getLevel() const {return m_level;}

      const std::string& getName() const { return m_name; }
      std::string ToYamlStr();
    private:
      std::string m_name;
      LogLevel::Level m_level;
      std::list<LogAppender::ptr> m_appenders; //Output to these apenders
      LogFormatter::ptr m_formatter; // default formatter
      MutexType m_mutex;
      Logger::ptr m_root;
  };

  class StdoutAppender: public LogAppender{ //Output to console
    public:
      typedef std::shared_ptr<StdoutAppender> ptr;
      StdoutAppender() {m_type = 2;}
      void log(
        std::shared_ptr<Logger> ptr,
        LogLevel::Level level, 
        LogEvent::ptr event
      ) override;
  };

  class FileAppender: public LogAppender{ //Output to file
    public:
      typedef std::shared_ptr<FileAppender> ptr;
      FileAppender(const std::string& filename);
      void log(
        std::shared_ptr<Logger> ptr,
        LogLevel::Level level, 
        LogEvent::ptr event
      ) override;
      bool reopen();
      const std::string getFileName() override {return m_filename;}
    private:
      std::string m_filename;
      std::ofstream m_filestream;
      uint64_t m_last_time = 0;
  };

  class LoggerManager {
    public:
      typedef Mutex MutexType;
      LoggerManager();
      Logger::ptr getLogger(const std::string& name);
      Logger::ptr getRoot() {return m_root;}
      void init();
    private:
      Logger::ptr m_root;
      MutexType m_mutex;
      std::map<std::string, Logger::ptr> m_loggers;
  };

  typedef SingletonPtr<LoggerManager> LoggerMgr;

}