
#ifndef HOST_LOG_H_
#define HOST_LOG_H_

enum LogLevel {

  // The service/app is going to stop or become unusable now.
  // An operator should definitely look into this soon.
	FATAL = 60,

  // Fatal for a particular request, but the service/app continues servicing other requests.
  // An operator should look at this soon(ish).
	ERROR = 50,

  // A note on something that should probably be looked at by an operator eventually.
	WARN = 40,

  // Detail on regular operation.
	INFO = 30,

  // Anything else, i.e. too verbose to be included in "info" level.
	DEBUG = 20,

  // Logging from external libraries used by your app or very detailed application logging.
	TRACE = 10,
};

namespace log {
  void SetLogLevel(LogLevel level);

  void Fatal(const char *format, ...);
  void Error(const char *format, ...);
  void Warn(const char *format, ...);
  void Info(const char *format, ...);
  void Debug(const char *format, ...);
  void Trace(const char *format, ...);
}

#endif
