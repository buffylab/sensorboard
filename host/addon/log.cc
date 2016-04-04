#include <sstream>
#include "log.h"

LogLevel g_log_level = LogLevel::TRACE;

static void Log(enum LogLevel level, const char *level_label, const char *format, va_list args) {
  std::stringstream str_stream;
  str_stream << "[" << level_label << "] " << format << "\n";

  const char* new_format = str_stream.str().c_str();
	vfprintf(level < LogLevel::ERROR ? stdout : stderr, new_format, args);
}

namespace log {
  void SetLogSevel(LogLevel level) {
    g_log_level = level;
  }

  void Fatal(const char *format, ...) {
    if(g_log_level > LogLevel::FATAL)
      return;

    va_list args;
  	va_start(args, format);
    Log(LogLevel::FATAL, "FATAL", format, args);
  	va_end(args);
  }

  void Error(const char *format, ...) {
    if(g_log_level < LogLevel::ERROR)
      return;

    va_list args;
  	va_start(args, format);
    Log(LogLevel::ERROR, "ERROR", format, args);
  	va_end(args);
  }

  void Warn(const char *format, ...) {
    if(g_log_level < LogLevel::WARN)
      return;

    va_list args;
  	va_start(args, format);
    Log(LogLevel::WARN, "WARN", format, args);
  	va_end(args);
  }

  void Info(const char *format, ...) {
    if(g_log_level < LogLevel::INFO)
      return;

    va_list args;
  	va_start(args, format);
    Log(LogLevel::INFO, "INFO", format, args);
  	va_end(args);
  }

  void Debug(const char *format, ...) {
    if(g_log_level < LogLevel::DEBUG)
      return;

    va_list args;
  	va_start(args, format);
    Log(LogLevel::DEBUG, "DEBUG", format, args);
  	va_end(args);
  }

  void Trace(const char *format, ...) {
    if(g_log_level < LogLevel::TRACE)
      return;

    va_list args;
  	va_start(args, format);
    Log(LogLevel::TRACE, "TRACE", format, args);
  	va_end(args);
  }
}
