//
// Created by janos on 19.02.20.
//

#pragma once


#include "fs.hpp"

#include <cstdint>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <cassert>
#include <mutex>
#include <fstream>


std::mutex g_mutex;


auto from_sv(std::string_view sv) {
    return std::make_pair(sv.data(), sv.data() + sv.size());
}

std::string_view to_sv(const char* const begin, const char* const end) {
    return std::string_view{begin, static_cast<std::size_t>(end - begin)};
}


void parse_threaded(std::string_view buf, std::vector<double>& slices) {
    auto [begin, end] = from_sv(buf);
    const char* curr = begin;
    std::vector<double> local_slices;
    std::uint32_t val = 0;
    while (curr != end) {
        if ('0' <= *curr && *curr <= '9') {
            val = val * 10 + (*curr - '0');
        }
        else if(val){
            local_slices.push_back(val);
            val = 0;
        }
        ++curr; // NOLINT
    }

    {
        std::lock_guard lock(g_mutex);
        slices.insert(slices.end(), local_slices.begin(), local_slices.end());
    }
}

std::vector<std::string_view> chunk(std::string_view whole, int n_chunks, char delim = '\n') {
    auto [whole_begin, whole_end] = from_sv(whole);
    auto        chunk_size        = (whole_end - whole_begin) / n_chunks;
    auto        chunks            = std::vector<std::string_view>{};
    const char* end               = whole_begin;
    for (int i = 0; end != whole_end && i < n_chunks; ++i) {
        const char* begin = end;
        if (i == n_chunks - 1) {
            end = whole_end; // always ensure last chunk goes to the end
        } else {
            end = std::min(begin + chunk_size, whole_end);   // NOLINT std::min for OOB check
            while (end != whole_end && *end != delim) ++end; // NOLINT ensure we have a whole line
            if (end != whole_end) ++end;                     // NOLINT one past the end
        }
        chunks.push_back(to_sv(begin, end));
    }
    return chunks;
}

void parse_input_threaded(
        const std::string& filename,
        std::vector<double>& slices,
        double& max_slices,
        std::size_t n_threads = std::thread::hardware_concurrency()) {
    auto mfile = os::fs::MemoryMappedFile{filename};
    auto threads = std::vector<std::thread>{};

    auto buffer = mfile.get_buffer();
    auto new_line = buffer.find('\n');
    auto first_line = std::string_view{buffer.data(), new_line};
    max_slices = static_cast<double>(strtol(first_line.data(), nullptr, 10));

    auto second_line = std::string_view{buffer.data() + new_line + 1,
                                        buffer.size() - new_line}; //remove new line at the end

    for (auto c: chunk(second_line, n_threads, ' ')){
        threads.emplace_back([&slices, c]{ parse_threaded(c, slices); });
    }
    for (auto& t: threads) t.join();
}


void parse_input(const std::string& filename){
    std::ifstream file(filename);
}
