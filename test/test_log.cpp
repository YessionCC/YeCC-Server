#include"log.hpp"
#include"util.hpp"
#include"config.hpp"
#include<iostream>
#include<thread>

int main() {
  auto logger = yecc::LoggerMgr::GetInstance()->getLogger("");
  // yecc::Logger::ptr logger(new yecc::Logger);
  // yecc::LogAppender::ptr appender1(new yecc::FileAppender("./log.txt"));
  // yecc::LogAppender::ptr appender2(new yecc::StdoutAppender());
  // appender1->setLevel(yecc::LogLevel::ERROR);
  // logger->addAppender(appender1);
  // logger->addAppender(appender2);
  YECC_LOG_ERROR(logger)<<"lalal";
  YECC_LOG_FMT_DEBUG(logger, "abs %s", "123");
  return 0;
}