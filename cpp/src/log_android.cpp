#include <planet/log.hpp>

#include <sstream>

#include <android/log.h>


namespace {


    constexpr char tag[] = "planet";


    int priority(planet::log::level const l) noexcept {
        switch (l) {
        case planet::log::level::debug: return ANDROID_LOG_DEBUG;
        case planet::log::level::info: return ANDROID_LOG_INFO;
        case planet::log::level::warning: return ANDROID_LOG_WARN;
        case planet::log::level::error: return ANDROID_LOG_ERROR;
        case planet::log::level::critical: return ANDROID_LOG_FATAL;
        }
        return ANDROID_LOG_INFO;
    }


    /**
     * Render a planet log message into logcat. The level and time stamp are
     * supplied to logcat separately (via the priority and its own clock), so
     * the text we emit is just the message payload plus the source location,
     * with no ANSI colour codes.
     */
    void to_logcat(planet::log::message const &m) noexcept {
        std::ostringstream os;
        planet::serialise::load_buffer lb{m.payload.cmemory()};
        planet::log::pretty_print_whole_buffer(os, lb);
        os << " (" << m.location.function_name() << ' '
           << m.location.file_name() << ':' << m.location.line() << ')';
        auto const s = os.str();
        __android_log_print(
                priority(m.level), tag, "%.*s", static_cast<int>(s.size()),
                s.data());
    }


    /**
     * Installed at `.so` load time. `planet-android` is linked with
     * `--whole-archive`, so this static initialiser is guaranteed to run
     * (exactly like the asset loader's `g_loader`) and redirects planet's
     * console output to logcat before any log messages are pumped.
     */
    bool const g_installed =
            (planet::log::console_output.store(&to_logcat), true);


}
