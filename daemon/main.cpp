#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <Carbon/Carbon.h>

#include <stdio.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include <functional>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "time_utils.h"
#include "string_utils.h"
#include "keymapping.h"
#include "log_utils.h"

typedef std::shared_mutex Lock;
typedef std::unique_lock<Lock> WriteLock;
typedef std::shared_lock<Lock> ReadLock;

namespace fs = std::filesystem;

Lock event_tap_lock;

struct Environment {
    std::string output_path;

    Environment(std::string);
};

Environment::Environment(std::string _output_path) {
    output_path = _output_path;
}

struct EventTap {
    CFMachPortRef handle;
    CFRunLoopSourceRef runloop_source;
    CGEventMask mask;
    // raw logs of each key stroke pressed
    std::vector<std::string> *logs;

    EventTap(void);
};

EventTap::EventTap() {
    logs = new std::vector<std::string>();
}

void log_current_keystrokes(
    struct Environment &env,
    struct EventTap &event_tap
) {
    ReadLock r_lock(event_tap_lock);
    // Aggregate locally
    std::unordered_map<std::string, int> histogram;
    int count = 0;
    for (auto key_name:(*event_tap.logs)) {
        if (histogram.find(key_name) == histogram.end()) {
            histogram[key_name] = 0;
        }
        histogram[key_name] += 1;
        count += 1;
    }

    auto data_dir = env.output_path.c_str();
    auto filename = format("%s/histogram_%s.txt", data_dir, get_today_date().c_str());

    log("Flushing to disk\n");

    if (!fs::exists(data_dir)) {
        fs::create_directory(data_dir);
    }

    // If we already have a file, reduce the contents
    if (fs::exists(filename)) {
        std::string line;
        std::ifstream histogram_file(filename);
        if (histogram_file.is_open()) {
            log("found existing file\n");
            while (getline(histogram_file, line)) {
                auto parts = split(line, ":|:");
                auto key_name = parts[0];
                auto count = std::stoi(parts[1]);
                histogram[key_name] += count;
            }
            histogram_file.close();
        }

    }

    // We write to a different file to avoid locking it for reads
    auto write_filename = format("%s/histogram_%s.out.txt", data_dir, get_today_date().c_str());
    std::ofstream histogram_write_file(write_filename);
    if (histogram_write_file.is_open()) {
        for (auto entry:histogram) {
            histogram_write_file << entry.first << ":|:" << entry.second << std::endl;
        }
        histogram_write_file.close();
    }

    // Overwite file
    if(rename(write_filename.c_str(), filename.c_str()) != 0) {
        log_error("failed to rename file: %s\n", write_filename.c_str());
    }

  event_tap.logs->clear();
}

CGEventRef on_keystroke(
    CGEventTapProxy proxy,
    CGEventType type,
    CGEventRef event,
    void *data
) {

    switch (type) {
        case kCGEventKeyDown: {

            auto key_code = CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
            auto flag = CGEventGetFlags(event);

            bool is_shift = get_is_shift(flag);

            auto key_str = key_code_to_str(key_code, is_shift, false);
            std::string prefix = "";
            if (get_is_cmd(flag)) {
                prefix += "[cmd] + ";
            }

            if (get_is_ctrl(flag)) {
                prefix += "[ctrl] + ";
            }

            // printf("key: %s%s\n", prefix.c_str(), key_str.c_str());
            struct EventTap *event_tap = (struct EventTap *) data;

            WriteLock w_lock(event_tap_lock);
            event_tap->logs->push_back(prefix + key_str);
            break;
        }
        // Right click from a logitech mouse. This is not part of CGEventType >.<
        case 14:
        break;

        default: {
            printf("Unknown type: %d\n", type);
        }
    }

    return event;
}

bool has_accessibility_privileges(struct Environment &env) {
    auto data_dir = env.output_path.c_str();
    auto no_permission_filename = format("%s/no_permission.txt", data_dir);

    // Only show prompt if file hasn't been created yet.
    CFBooleanRef show_prompt;
    if (fs::exists(no_permission_filename.c_str())) {
        show_prompt = kCFBooleanTrue;
    } else {
        show_prompt = kCFBooleanFalse;
    }

    bool result;
    const void *keys[] = { kAXTrustedCheckOptionPrompt };
    const void *values[] = { show_prompt };
    CFDictionaryRef options = CFDictionaryCreate(
        kCFAllocatorDefault,
        keys,
        values,
        sizeof(keys) / sizeof(*keys),
        &kCFCopyStringDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks
    );

    result = AXIsProcessTrustedWithOptions(options);
    CFRelease(options);

    if (!result) {
        // Create a file to remember not to ask next time. This is important when this
        // binary is being scheduled in a daemon
        std::ofstream output(no_permission_filename);
    } else {
        remove(no_permission_filename.c_str());
    }

    return result;
}

bool is_tab_event_enabled(struct EventTap *event_tap) {
    return event_tap->handle && CGEventTapIsEnabled(event_tap->handle);
}


bool listen_to_keystrokes(
    struct Environment &env,
    struct EventTap &event_tap
) {

    event_tap.mask = (1 << kCGEventKeyDown) | (1 << NX_SYSDEFINED);

    event_tap.handle = CGEventTapCreate(
        kCGSessionEventTap,
        kCGHeadInsertEventTap,
        kCGEventTapOptionDefault,
        event_tap.mask,
        on_keystroke,
        &event_tap
    );

    if (!is_tab_event_enabled(&event_tap)) {
        return false;
    }

    event_tap.runloop_source = CFMachPortCreateRunLoopSource(
        kCFAllocatorDefault,
        event_tap.handle,
        0
    );
    CFRunLoopAddSource(CFRunLoopGetMain(), event_tap.runloop_source, kCFRunLoopCommonModes);

    // Log the total keystrokes
    std::thread([&env, &event_tap]() {
        while (true) {
            auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(5000);
            std::this_thread::sleep_until(x);
            log_current_keystrokes(env, event_tap);
        }
    }).detach();

    return true;
}

void show_help() {
    log_error("Usage: main -o <dir where to output data>\n");
}

int main(int argc, char **argv) {

    if (argc != 3) {
        show_help();
    }

    std::string param_name(argv[1]);
    std::string expected_param("-o");
    if (param_name != expected_param) {
        show_help();
    }

    std::string output_path(argv[2]);
    std::cout << "output: " << output_path << std::endl;
    struct Environment env = Environment(output_path);

    log("Starting program at %s\n", fs::current_path().c_str());

    if (!has_accessibility_privileges(env)) {
        log_error("must be run with accessibility access!\n");
    }

    struct EventTap event_tap = EventTap();

    if (!listen_to_keystrokes(env, event_tap)) {
        printf("Failed to register keystroke listener\n");
    }

    CFRunLoopRun();
    return 0;
}
