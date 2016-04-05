#include <stdarg.h>
#include <sstream>
#include "log.h"

LogLevel g_log_level = LogLevel::LL_TRACE;

static void Log(enum LogLevel level, const char *level_label, const char *format, va_list args) {
  std::stringstream str_stream;
  str_stream << "[" << level_label << "] " << format << "\n";

  const char* new_format = str_stream.str().c_str();
	vfprintf(level < LogLevel::LL_ERROR ? stdout : stderr, new_format, args);
}

namespace nlog {
  void SetLogSevel(LogLevel level) {
    g_log_level = level;
  }

  void Fatal(const char *format, ...) {
    if(g_log_level > LogLevel::LL_FATAL)
      return;

    va_list args;
  	va_start(args, format);
    Log(LogLevel::LL_FATAL, "FATAL", format, args);
  	va_end(args);
  }

  void Error(const char *format, ...) {
    if(g_log_level > LogLevel::LL_ERROR)
      return;

    va_list args;
  	va_start(args, format);
    Log(LogLevel::LL_ERROR, "ERROR", format, args);
  	va_end(args);
  }

  void Warn(const char *format, ...) {
    if(g_log_level > LogLevel::LL_WARN)
      return;

    va_list args;
  	va_start(args, format);
    Log(LogLevel::LL_WARN, "WARN", format, args);
  	va_end(args);
  }

  void Info(const char *format, ...) {
    if(g_log_level > LogLevel::LL_INFO)
      return;

    va_list args;
  	va_start(args, format);
    Log(LogLevel::LL_INFO, "INFO", format, args);
  	va_end(args);
  }

  void Debug(const char *format, ...) {
    if(g_log_level > LogLevel::LL_DEBUG)
      return;

    va_list args;
  	va_start(args, format);
    Log(LogLevel::LL_DEBUG, "DEBUG", format, args);
  	va_end(args);
  }

  void Trace(const char *format, ...) {
    if(g_log_level > LogLevel::LL_TRACE)
      return;

    va_list args;
  	va_start(args, format);
    Log(LogLevel::LL_TRACE, "TRACE", format, args);
  	va_end(args);
  }
}
