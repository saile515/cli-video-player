#include <chrono>
#include <filesystem>
#include <fmt/core.h>
#include <iostream>
#include <math.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <stdio.h>
#include <string>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>

static std::string ansi_string(int r, int g, int b)
{
    return fmt::format("\033[38;2;{};{};{}m", r, g, b);
}

static void print_color(int r, int g, int b, std::string str)
{
    fmt::print("{}{}", ansi_string(r, g, b), str);
}

int main(int argc, char** argv)
{
    if (argc >= 2) {
        if (!std::filesystem::exists(argv[1])) {
            fmt::print("Error: File \"{}\" does not exist\n", argv[1]);
            return -1;
        }
    } else {
        fmt::print("Error: No file specified\n");
        return -1;
    }

    cv::VideoCapture video(argv[1]);

    if (!video.isOpened()) {
        fmt::print("Error opening video stream or file\n");
        return -1;
    }

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int window_width = std::floor(w.ws_col / 2);
    int window_height = w.ws_row - 1;

    double fps = video.get(5);
    int frame_count = video.get(7);

    std::chrono::time_point last_frame = std::chrono::high_resolution_clock::now();

    while (video.isOpened()) {
        cv::Mat frame;
        int frame_width = frame.cols;
        int frame_height = frame.rows;

        bool isSuccess = video.read(frame);
        cv::resize(frame, frame, cv::Size(window_width, window_height));

        if (isSuccess == true) {
            std::string out = "\033[1;1H";
            for (int y = 0; y < window_height; ++y) {
                for (int x = 0; x < window_width; ++x) {
                    auto pixel = frame.at<cv::Vec3b>(y, x);
                    out += ansi_string(pixel[0], pixel[1], pixel[2]) + "██";
                }
                out += "\n";
            }

            write(1, out.c_str(), out.size());
        }

        if (isSuccess == false) {
            break;
        }

        std::chrono::time_point time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::milliseconds>(last_frame - time);

        if (time_span.count() < (1000 / fps)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(int((1000.0 / fps) - time_span.count())));
        }

        last_frame = std::chrono::high_resolution_clock::now();
    }

    video.release();
}
