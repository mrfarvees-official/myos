#include "graphics/Framebuffer.hpp"

#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace myos::graphics {
    Framebuffer::Framebuffer()
        : fd_(-1),
          memory_(nullptr),
          memorySize_(0),
          width_(0),
          height_(0),
          bitsPerPixel_(0),
          lineLength_(0)
    {
    }

    Framebuffer::~Framebuffer()
    {
        if (memory_ != nullptr) {
            munmap(
                memory_,
                memorySize_
            );
        }

        if (fd_ >= 0) {
            ::close(fd_);
        }
    }

    bool Framebuffer::open()
    {
        fd_ = ::open("/dev/fb0", O_RDWR);

        if (fd_ < 0) {
            std::cerr << "[desktop] Cannot open /dev/fb0\n";
            return false;
        }

        fb_fix_screeninfo fixedInfo {};
        fb_var_screeninfo variableInfo {};

        if (ioctl(fd_, FBIOGET_FSCREENINFO, &fixedInfo) < 0) {
            std::cerr << "[desktop] FBIOGET_FSCREENINFO failed\n";
            return false;
        }

        if (ioctl(fd_, FBIOGET_VSCREENINFO, &variableInfo) < 0) {
            std::cerr << "[desktop] FBIOGET_VSCREENINFO failed\n";
            return false;
        }

        width_ = static_cast<int>(variableInfo.xres);
        height_ = static_cast<int>(variableInfo.yres);
        bitsPerPixel_ = static_cast<int>(variableInfo.bits_per_pixel);
        lineLength_ = static_cast<int>(fixedInfo.line_length);
        memorySize_ = static_cast<std::size_t>(fixedInfo.smem_len);
        memory_ = static_cast<uint8_t*>(
            mmap(nullptr, memorySize_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0)
        );

        if (memory_ == MAP_FAILED) {
            memory_ = nullptr;
            std::cerr << "[desktop] mmap framebuffer failed\n";
            return false;
        }

        std::cout 
            << "[desktop] Framebuffer detected\n"
            << "[desktop] Resolution: "
            << width_
            << "x"
            << height_
            << "\n"
            << "[desktop] Bits per pixel: "
            << bitsPerPixel_
            << "\n";

        return true;
    }

    int Framebuffer::width() const
    {
        return width_;
    }

    int Framebuffer::height() const
    {
        return height_;
    }

    int Framebuffer::bitsPerPixel() const
    {
        return bitsPerPixel_;
    }

    bool Framebuffer::present(
        const Surface& surface
    )
    {
        if (memory_ == nullptr) {
            return false;
        }

        if (bitsPerPixel_ != 32) {
            std::cerr
                << "[desktop] Currently only 32-bit framebuffer is supported\n";

            return false;
        }

        const int copyWidth =
            std::min(width_, surface.width());

        const int copyHeight =
            std::min(height_, surface.height());

        const std::size_t rowBytes =
            static_cast<std::size_t>(copyWidth) * 4;

        if (
            copyWidth == width_ &&
            copyWidth == surface.width() &&
            lineLength_ == static_cast<int>(rowBytes)
        ) {
            const std::size_t totalBytes =
                rowBytes *
                static_cast<std::size_t>(copyHeight);

            std::memcpy(
                memory_,
                surface.pixels(),
                totalBytes
            );

            return true;
        }

        for (
            int y = 0;
            y < copyHeight;
            ++y
        ) {
            uint8_t* destination =
                memory_ +
                static_cast<std::size_t>(y) *
                static_cast<std::size_t>(lineLength_);

            const uint8_t* source =
                reinterpret_cast<const uint8_t*>(
                    surface.pixels() +
                    static_cast<std::size_t>(y) *
                    static_cast<std::size_t>(
                        surface.width()
                    )
                );

            std::memcpy(
                destination,
                source,
                rowBytes
            );
        }

        return true;
    }
    
}