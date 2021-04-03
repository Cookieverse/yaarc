#ifndef YAARC_LOGLEVEL_HPP
#define YAARC_LOGLEVEL_HPP
namespace  yaarc {
	enum class LogLevel{
		Debug,
		Info,
		Warning,
		Error
	};

	const char* LogLevelToString(LogLevel level) {
		switch (level){
			case LogLevel::Debug:
				return "debug";
			case LogLevel::Info:
				return "info";
			case LogLevel::Warning:
				return "warning";
			case LogLevel::Error:
				return "error";
		}
		return "n/a";
	}
}
#endif //YAARC_LOGLEVEL_HPP
