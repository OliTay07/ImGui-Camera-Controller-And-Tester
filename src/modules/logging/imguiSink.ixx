module;

#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/pattern_formatter.h>

#ifdef _DEBUG
#include <Windows.h>
#define DEBUG_PRINT(msg) OutputDebugStringA(msg)
#else
#define DEBUG_PRINT(msg) // Do nothing in release builds
#endif

export module imguiSink;

export namespace imguiSink
{
    // ImGui Sink using its own mutex
    export class ImGuiSink : public spdlog::sinks::base_sink<std::mutex>
    {
    public:
        ImGuiSink() = default;
        ~ImGuiSink() = default;

        // Retrieve the log entries
        const std::vector<std::string>& get_logs() const;

        // Clear the logs
        void clear();

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override;
        void flush_() override;

    private:
        mutable std::recursive_mutex logs_mutex_;
        std::vector<std::string> logs_;
    };

    // Implementations

    const std::vector<std::string>& ImGuiSink::get_logs() const
    {
        std::lock_guard<std::recursive_mutex> lock(logs_mutex_);
        DEBUG_PRINT(("get_logs called. Total logs: " + std::to_string(logs_.size()) + "\n").c_str());
        return logs_;
    }

    void ImGuiSink::clear()
    {
        std::lock_guard<std::recursive_mutex> lock(logs_mutex_);
        logs_.clear();
    }

    void ImGuiSink::sink_it_(const spdlog::details::log_msg& msg)
    {
        try
        {
            spdlog::memory_buf_t formatted;
            this->formatter_->format(msg, formatted);

            std::lock_guard<std::recursive_mutex> lock(logs_mutex_);
            logs_.emplace_back(std::string(formatted.data(), formatted.size()));
        }
        catch (const std::exception& e)
        {
            DEBUG_PRINT(("Exception in sink_it_: " + std::string(e.what()) + "\n").c_str());
        }
        catch (...)
        {
            DEBUG_PRINT("Unknown exception in sink_it_\n");
        }
    }

    void ImGuiSink::flush_()
    {
        // No need to implement flush for ImGui
    }

    void init_logger()
    {
        try
        {
            // Create a shared pointer to the custom sink
            auto imgui_sink = std::make_shared<ImGuiSink>();

            // Set a simple formatter for the sink
            imgui_sink->set_formatter(std::make_unique<spdlog::pattern_formatter>("[%H:%M:%S] [%l] %v"));


            // Create the logger with the custom sink
            auto logger = std::make_shared<spdlog::logger>("IMGUI_LOGGER", imgui_sink);

            // Register the logger with spdlog
            spdlog::register_logger(logger);

            // Set the default log level and flush policy
            logger->set_level(spdlog::level::trace);
            logger->flush_on(spdlog::level::trace);

            // Log an initialization message
            logger->info("Logger 'IMGUI_LOGGER' initialized with ImGuiSink.");
        }
        catch (const std::exception& e)
        {
            DEBUG_PRINT(("Exception in init_logger: " + std::string(e.what()) + "\n").c_str());
        }
    }

} // namespace imguiSink
