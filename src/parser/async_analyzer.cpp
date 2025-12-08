/**
 * Async/Coroutine Analyzer
 * Analyzes C++20 coroutines and async patterns for conversion to Rust async/await and Go goroutines
 */

#include "ir.h"
#include <regex>
#include <algorithm>
#include <set>

namespace hybrid {

/**
 * Async/Coroutine Analyzer
 * Detects co_await, co_return, co_yield, std::future, std::async, etc.
 */
class AsyncAnalyzer {
public:
    /**
     * Analyze function for coroutine patterns
     */
    void analyzeFunction(Function& func) {
        // Detect coroutine keywords
        detectCoroutineKeywords(func);

        // Detect std::future and std::promise usage
        detectFuturePromise(func);

        // Detect std::async calls
        detectAsyncCalls(func);

        // Determine if function is async
        func.is_async = func.coroutine_info.is_coroutine ||
                       !func.futures.empty() ||
                       !func.async_tasks.empty();
    }

private:
    /**
     * Detect C++20 coroutine keywords: co_await, co_return, co_yield
     */
    void detectCoroutineKeywords(Function& func) {
        std::string body = func.body;
        CoroutineInfo& coro_info = func.coroutine_info;

        // Pattern 1: co_await expression
        std::regex await_pattern(R"(co_await\s+([^;]+))");

        // Pattern 2: co_return expression
        std::regex return_pattern(R"(co_return\s+([^;]+))");

        // Pattern 3: co_yield expression
        std::regex yield_pattern(R"(co_yield\s+([^;]+))");

        std::smatch match;
        std::string::const_iterator search_start(body.cbegin());

        // Search for co_await
        search_start = body.cbegin();
        while (std::regex_search(search_start, body.cend(), match, await_pattern)) {
            AsyncOperation op;
            op.op_type = AsyncOpType::CoAwait;
            op.expression = trim(match[1].str());

            coro_info.async_operations.push_back(op);
            coro_info.uses_co_await = true;
            search_start = match.suffix().first;
        }

        // Search for co_return
        search_start = body.cbegin();
        while (std::regex_search(search_start, body.cend(), match, return_pattern)) {
            AsyncOperation op;
            op.op_type = AsyncOpType::CoReturn;
            op.expression = trim(match[1].str());

            coro_info.async_operations.push_back(op);
            coro_info.uses_co_return = true;
            search_start = match.suffix().first;
        }

        // Search for co_yield
        search_start = body.cbegin();
        while (std::regex_search(search_start, body.cend(), match, yield_pattern)) {
            AsyncOperation op;
            op.op_type = AsyncOpType::CoYield;
            op.expression = trim(match[1].str());

            coro_info.async_operations.push_back(op);
            coro_info.uses_co_yield = true;
            coro_info.is_generator = true;  // co_yield makes it a generator
            search_start = match.suffix().first;
        }

        // Mark as coroutine if any coroutine keyword is used
        if (coro_info.uses_co_await || coro_info.uses_co_return || coro_info.uses_co_yield) {
            coro_info.is_coroutine = true;
        }
    }

    /**
     * Detect std::future and std::promise usage
     */
    void detectFuturePromise(Function& func) {
        std::string body = func.body;

        // Pattern 1: std::future<T> var = ...
        std::regex future_pattern(
            R"(std::future<([^>]+)>\s+(\w+)\s*(?:=|;))"
        );

        // Pattern 2: std::promise<T> var
        std::regex promise_pattern(
            R"(std::promise<([^>]+)>\s+(\w+))"
        );

        // Pattern 3: future.get()
        std::regex get_pattern(R"((\w+)\.get\(\))");

        std::smatch match;
        std::string::const_iterator search_start(body.cbegin());

        // Search for future declarations
        search_start = body.cbegin();
        while (std::regex_search(search_start, body.cend(), match, future_pattern)) {
            FutureInfo future_info;
            future_info.future_var_name = match[2].str();

            // Create type for future value
            auto value_type = std::make_shared<Type>(TypeKind::Void);
            value_type->name = match[1].str();
            future_info.value_type = value_type;

            func.futures.push_back(future_info);
            search_start = match.suffix().first;
        }

        // Search for promise declarations
        search_start = body.cbegin();
        while (std::regex_search(search_start, body.cend(), match, promise_pattern)) {
            std::string promise_var = match[2].str();
            std::string value_type_name = match[1].str();

            // Associate promise with future if exists
            for (auto& future : func.futures) {
                if (future.promise_var_name.empty()) {
                    future.promise_var_name = promise_var;
                    break;
                }
            }

            search_start = match.suffix().first;
        }
    }

    /**
     * Detect std::async calls
     */
    void detectAsyncCalls(Function& func) {
        std::string body = func.body;

        // Pattern 1: auto result = std::async(func, args...)
        std::regex async_pattern1(
            R"((?:auto|std::future<[^>]+>)\s+(\w+)\s*=\s*std::async\s*\(\s*([^,)]+)(?:,\s*([^)]*))?\))"
        );

        // Pattern 2: std::async(std::launch::*, func, args...)
        std::regex async_pattern2(
            R"(std::async\s*\(\s*std::launch::\w+\s*,\s*([^,)]+)(?:,\s*([^)]*))?\))"
        );

        std::smatch match;
        std::string::const_iterator search_start(body.cbegin());

        // Search for async pattern 1
        search_start = body.cbegin();
        while (std::regex_search(search_start, body.cend(), match, async_pattern1)) {
            AsyncTaskInfo task_info;
            task_info.task_var_name = match[1].str();
            task_info.async_function_name = trim(match[2].str());

            // Parse arguments
            if (match.size() > 3 && match[3].matched) {
                parseArguments(match[3].str(), task_info.arguments);
            }

            func.async_tasks.push_back(task_info);
            search_start = match.suffix().first;
        }

        // Search for async pattern 2
        search_start = body.cbegin();
        while (std::regex_search(search_start, body.cend(), match, async_pattern2)) {
            AsyncTaskInfo task_info;
            task_info.async_function_name = trim(match[1].str());

            // Parse arguments
            if (match.size() > 2 && match[2].matched) {
                parseArguments(match[2].str(), task_info.arguments);
            }

            task_info.detached = true;  // No variable assignment means detached
            func.async_tasks.push_back(task_info);
            search_start = match.suffix().first;
        }
    }

    /**
     * Parse comma-separated arguments
     */
    void parseArguments(const std::string& args_str, std::vector<std::string>& arguments) {
        if (args_str.empty()) return;

        std::string current_arg;
        int paren_depth = 0;
        int angle_depth = 0;

        for (char c : args_str) {
            if (c == '(' || c == '[' || c == '{') {
                paren_depth++;
            } else if (c == ')' || c == ']' || c == '}') {
                paren_depth--;
            } else if (c == '<') {
                angle_depth++;
            } else if (c == '>') {
                angle_depth--;
            }

            if (c == ',' && paren_depth == 0 && angle_depth == 0) {
                arguments.push_back(trim(current_arg));
                current_arg.clear();
            } else {
                current_arg += c;
            }
        }

        if (!current_arg.empty()) {
            arguments.push_back(trim(current_arg));
        }
    }

    /**
     * Trim whitespace from string
     */
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }
};

} // namespace hybrid
