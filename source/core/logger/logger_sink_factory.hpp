#ifndef _LOGGER_SINK_FACTORY_H_
#define _LOGGER_SINK_FACTORY_H_

/*
    How to add a new sink :

    1. Add a new class in logger_sinks directory by taking example of existing sink classes
    2. Add its include in this file
    3. Increment SINK_NUMBER const integer in this file
    4. Add it to static array LOGGER_SINKS in this file
    5. Add a new entry for your class in LoggerSinkFactory::create in this file
*/

#include <core/logger/base_logger_sink.hpp>

#include <string>
#include <array>
#include <memory>

#include <core/design_patterns/factory.hpp>

// SINK INCLUDES
#include <core/logger/file_sink.hpp>
#include <core/logger/console_sink.hpp>
#include <core/logger/shared_memory_sink.hpp>

namespace core
{

// SINK NUMBER
const int SINK_NUMBER = 3;

// SINK ARRAY
const std::array<const char*, SINK_NUMBER> LOGGER_SINKS = {
                                                                FILE_SINK,
                                                                CONSOLE_SINK,
                                                                SHARED_MEMORY_SINK
                                                          };

class LoggerSinkFactory : Factory<LoggerSinkPtr>
{
    public :
        LoggerSinkPtr create(const std::string& sinkName) override
        {
            BaseLoggerSink* product{nullptr};

            if (sinkName.compare(FILE_SINK) == 0)
            {
                product = new FileSink();
            }
            else if (sinkName.compare(CONSOLE_SINK) == 0)
            {
                product = new ConsoleSink();
            }
            else if (sinkName.compare(SHARED_MEMORY_SINK) == 0)
            {
                product = new SharedMemorySink();
            }

            return LoggerSinkPtr(product);
        }

        static bool isValidSink(const std::string& sinkName)
        {
            for (auto& sink : LOGGER_SINKS)
            {
                if (sinkName.compare(sink) == 0)
                {
                    return true;
                }
            }

            return false;
        }
};



} // namespace

#endif