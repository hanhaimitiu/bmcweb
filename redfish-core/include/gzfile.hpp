// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include "logging.hpp"

#include <zlib.h>

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

class GzFileReader
{
  public:
    bool gzGetLines(const std::string& filename, uint64_t skip, uint64_t top,
                    std::vector<std::string>& logEntries, size_t& logCount)
    {
        gzFile logStream = gzopen(filename.c_str(), "r");
        if (logStream == nullptr)
        {
            BMCWEB_LOG_ERROR("Can't open gz file: {}", filename);
            return false;
        }

        if (!readFile(logStream, skip, top, logEntries, logCount))
        {
            gzclose(logStream);
            return false;
        }
        gzclose(logStream);
        return true;
    }

    std::string getLastMessage()
    {
        return lastMessage;
    }

  private:
    std::string lastMessage;
    std::string lastDelimiter;
    size_t totalFilesSize = 0;

    static void printErrorMessage(gzFile logStream)
    {
        int errNum = 0;
        const char* errMsg = gzerror(logStream, &errNum);

        BMCWEB_LOG_ERROR(
            "Error reading gz compressed data.\nError Message: {}\nError Number: {}",
            errMsg, errNum);
    }

    bool readFile(gzFile logStream, uint64_t skip, uint64_t top,
                  std::vector<std::string>& logEntries, size_t& logCount)
    {
        constexpr int bufferLimitSize = 1024;
        do
        {
            std::string bufferStr;
            bufferStr.resize(bufferLimitSize);

            int bytesRead = gzread(logStream, bufferStr.data(),
                                   static_cast<unsigned int>(bufferStr.size()));
            // On errors, gzread() shall return a value less than 0.
            // 出错时，gzread() 应返回小于 0 的值。
            if (bytesRead < 0)
            {
                printErrorMessage(logStream);
                return false;
            }
            bufferStr.resize(static_cast<size_t>(bytesRead));
            if (!hostLogEntryParser(bufferStr, skip, top, logEntries, logCount))
            {
                BMCWEB_LOG_ERROR("Error occurs during parsing host log.");
                return false;
            }
        } while (gzeof(logStream) != 1);

        return true;
    }

    bool hostLogEntryParser(const std::string& bufferStr, uint64_t skip,
                            uint64_t top, std::vector<std::string>& logEntries,
                            size_t& logCount)
    {
        // Assume we have 8 files, and the max size of each file is
        // 16k, so define the max size as 256kb (double of 8 files *
        // 16kb)
        // 假设有 8 个文件，每个文件最大为
        // 16k，因此将最大大小定义为 256kb（8 个文件 * 16kb 的
        // 两倍）
        constexpr size_t maxTotalFilesSize = 262144;

        // It may contain several log entry in one line, and
        // the end of each log entry will be '\r\n' or '\r'.
        // So we need to go through and split string by '\n' and '\r'
        // 一行中可能包含多条日志条目，且
        // 每条日志条目的结尾是 '\r\n' 或 '\r'。
        // 因此需要遍历并按 '\n' 和 '\r' 拆分字符串
        size_t pos = bufferStr.find_first_of("\n\r");
        size_t initialPos = 0;
        std::string newLastMessage;

        while (pos != std::string::npos)
        {
            std::string logEntry =
                bufferStr.substr(initialPos, pos - initialPos);
            // Since there might be consecutive delimiters like "\r\n", we need
            // to filter empty strings.
            // 由于可能存在像 "\r\n" 这样的连续分隔符，我们需要
            // 过滤掉空字符串。
            if (!logEntry.empty())
            {
                logCount++;
                if (!lastMessage.empty())
                {
                    logEntry.insert(0, lastMessage);
                    lastMessage.clear();
                }
                if (logCount > skip && logCount <= (skip + top))
                {
                    totalFilesSize += logEntry.size();
                    if (totalFilesSize > maxTotalFilesSize)
                    {
                        BMCWEB_LOG_ERROR(
                            "File size exceeds maximum allowed size of {}",
                            maxTotalFilesSize);
                        return false;
                    }
                    logEntries.push_back(logEntry);
                }
            }
            else
            {
                // Handle consecutive delimiter. '\r\n' act as a single
                // delimiter, the other case like '\n\n', '\n\r' or '\r\r' will
                // push back a "\n" as a log.
                // 处理连续分隔符。'\r\n' 视为单个
                // 分隔符，其他情况如 '\n\n'、'\n\r' 或 '\r\r' 会
                // 将 "\n" 作为一条日志推入。
                std::string delimiters;
                if (pos > 0)
                {
                    delimiters = bufferStr.substr(pos - 1, 2);
                }
                // Handle consecutive delimiter but spilt between two files.
                // 处理连续分隔符被拆分到两个文件之间的情况。
                if (pos == 0 && !(lastDelimiter.empty()))
                {
                    delimiters = lastDelimiter + bufferStr.substr(0, 1);
                }
                if (delimiters != "\r\n")
                {
                    logCount++;
                    if (logCount > skip && logCount <= (skip + top))
                    {
                        totalFilesSize++;
                        if (totalFilesSize > maxTotalFilesSize)
                        {
                            BMCWEB_LOG_ERROR(
                                "File size exceeds maximum allowed size of {}",
                                maxTotalFilesSize);
                            return false;
                        }
                        logEntries.emplace_back("\n");
                    }
                }
            }
            initialPos = pos + 1;
            pos = bufferStr.find_first_of("\n\r", initialPos);
        }

        // Store the last message
        // 存储最后一条消息
        if (initialPos < bufferStr.size())
        {
            newLastMessage = bufferStr.substr(initialPos);
        }
        // If consecutive delimiter spilt by buffer or file, the last character
        // must be the delimiter.
        // 如果连续分隔符被缓冲区或文件拆开，则最后一个字符
        // 必定是分隔符。
        else if (initialPos == bufferStr.size())
        {
            lastDelimiter = std::string(1, bufferStr.back());
        }
        // If file doesn't contain any "\r" or "\n", initialPos should be zero
        // 如果文件不包含任何 "\r" 或 "\n"，initialPos 应为零
        if (initialPos == 0)
        {
            // Solved an edge case that the log doesn't in skip and top range,
            // but consecutive files don't contain a single delimiter, this
            // lastMessage becomes unnecessarily large. Since last message will
            // prepend to next log, logCount need to plus 1
            // 解决一个边界情况：日志不在 skip 和 top 范围内，
            // 但连续的文件中都不包含任何分隔符，导致
            // lastMessage 变得不必要地大。由于最后一条消息会
            // 拼接到下一条日志前，logCount 需要加 1
            if ((logCount + 1) > skip && (logCount + 1) <= (skip + top))
            {
                lastMessage.insert(
                    lastMessage.end(),
                    std::make_move_iterator(newLastMessage.begin()),
                    std::make_move_iterator(newLastMessage.end()));

                // Following the previous question, protect lastMessage don't
                // larger than max total files size
                // 接着上面的问题，防止 lastMessage
                // 超过文件总大小上限
                size_t tmpMessageSize = totalFilesSize + lastMessage.size();
                if (tmpMessageSize > maxTotalFilesSize)
                {
                    BMCWEB_LOG_ERROR(
                        "File size exceeds maximum allowed size of {}",
                        maxTotalFilesSize);
                    return false;
                }
            }
        }
        else
        {
            if (!newLastMessage.empty())
            {
                lastMessage = std::move(newLastMessage);
            }
        }
        return true;
    }

  public:
    GzFileReader() = default;
    ~GzFileReader() = default;
    GzFileReader(const GzFileReader&) = delete;
    GzFileReader& operator=(const GzFileReader&) = delete;
    GzFileReader(GzFileReader&&) = delete;
    GzFileReader& operator=(GzFileReader&&) = delete;
};
