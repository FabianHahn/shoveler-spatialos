#include <stdbool.h> // bool
#include <stdio.h> // FILE, fprintf, fflush
#include <stdlib.h> // free
#include <stddef.h> // NULL
#include <string.h> // strdup, strstr

#include <glib.h>

#include "shoveler/spatialos/log.h"

static const char *logLocationPrefix = "workers/cmake/";

static void logHandler(const char *file, int line, ShovelerSpatialosLogLevel level, const char *message);
static bool shouldLog(ShovelerSpatialosLogLevel level);
static char *formatLogMessage(const char *file, int line, ShovelerSpatialosLogLevel level, const char *message);
static const char *getStaticLogLevelName(ShovelerSpatialosLogLevel level);

static ShovelerSpatialosLogLevel logLevel;
static FILE *logChannel;

void shovelerSpatialosLogInit(ShovelerSpatialosLogLevel level, FILE *channel)
{
	logLevel = level;
	logChannel = channel;
}

void shovelerSpatialosLogMessage(const char *file, int line, ShovelerSpatialosLogLevel level, const char *message, ...)
{
	va_list va;
	va_start(va, message);
	GString *assembled = g_string_new("");
	g_string_append_vprintf(assembled, message, va);
	logHandler(file, line, level, assembled->str);
	g_string_free(assembled, true);
}

static void logHandler(const char *file, int line, ShovelerSpatialosLogLevel level, const char *message)
{
	if(shouldLog(level)) {
		char *formatted = formatLogMessage(file, line, level, message);
		fprintf(logChannel, "%s\n", formatted);
		g_free(formatted);
	}
	fflush(logChannel);
}

static bool shouldLog(ShovelerSpatialosLogLevel level)
{
	return logLevel & level;
}

static char *formatLogMessage(const char *file, int line, ShovelerSpatialosLogLevel level, const char *message)
{
	const char *strippedLocation = strstr(file, logLocationPrefix);
	if(strippedLocation != NULL) {
		strippedLocation += strlen(logLocationPrefix);
	} else {
		strippedLocation = file;
	}

	GDateTime *now = g_date_time_new_now_local();
	GString *result = g_string_new("");

	g_string_append_printf(result, "[%02d:%02d:%02d] (%s:%s:%d) %s", g_date_time_get_hour(now), g_date_time_get_minute(now), g_date_time_get_second(now), getStaticLogLevelName(level), strippedLocation, line, message);

	g_date_time_unref(now);
	return g_string_free(result, false);
}

static const char *getStaticLogLevelName(ShovelerSpatialosLogLevel level)
{
	if(level & SHOVELER_SPATIALOS_LOG_LEVEL_TRACE) {
		return "trace";
	} else if(level & SHOVELER_SPATIALOS_LOG_LEVEL_INFO) {
		return "info";
	} else if(level & SHOVELER_SPATIALOS_LOG_LEVEL_WARNING) {
		return "warning";
	} else if(level & SHOVELER_SPATIALOS_LOG_LEVEL_ERROR) {
		return "error";
	} else {
		return "unknown";
	}
}
