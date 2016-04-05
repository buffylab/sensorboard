
#ifndef HOST_LOG_H_
#define HOST_LOG_H_

enum LogLevel {

  // The service/app is going to stop or become unusable now.
  // An operator should definitely look into this soon.
  LL_FATAL = 60,

  // Fatal for a particular request, but the service/app continues servicing other requests.
  // An operator should look at this soon(ish).
  LL_ERROR = 50,

  // A note on something that should probably be looked at by an operator eventually.
  LL_WARN = 40,

  // Detail on regular operation.
  LL_INFO = 30,

  // Anything else, i.e. too verbose to be included in "info" level.
  LL_DEBUG = 20,

  // Logging from external libraries used by your app or very detailed application logging.
  LL_TRACE = 10
};

namespace nlog {
  void SetLogLevel(LogLevel level);

  void Fatal(const char *format, ...);
  void Error(const char *format, ...);
  void Warn(const char *format, ...);
  void Info(const char *format, ...);
  void Debug(const char *format, ...);
  void Trace(const char *format, ...);
}

#endif
