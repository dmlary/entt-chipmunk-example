#include <stb_image.h>
#include <errno.h>
#include "log.hpp"
#include "fmt/std.hpp"
#include "image.hpp"

Image::Image(const std::filesystem::path &path)
    : m_label{path.string()}
{
    int bpp, width, height;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &bpp, 4);
    if (data == nullptr) {
        log_error("stbi_load(\"{}\") failed; {}; {}",
                path, stbi_failure_reason(), strerror(errno));
        return;
    }
    m_label = path.string();
    m_data = std::make_unique<unsigned char *>(data);
    make_image(bpp * width * height, width, height);
}

Image::Image(
        const std::string &label,
        const void *data,
        size_t size,
        size_t width,
        size_t height)
    : m_label{label}
{
    m_data = std::make_unique<unsigned char *>(new uint8_t[size]);
    memcpy(*m_data, data, size);
    make_image(size, width, height);
}

Image::~Image()
{
    reset();
}

Image::Image(Image&& other)
{
    *this = std::move(other);
}

Image&
Image::operator=(Image&& other)
{
    if (this != &other) {
        m_label = other.m_label;
        m_data = std::move(other.m_data);
        m_desc = other.m_desc;
        m_desc.label = m_label.c_str();
        m_image = other.m_image;

        other.m_label = "[object moved]";
        other.m_data = nullptr;
        other.m_desc = {};
        other.m_image = {};
    }
    return *this;
}

void
Image::reset()
{
    if (m_image.id != SG_INVALID_ID) {
        log_debug("destroy sg_image {}", m_image.id);
        sg_destroy_image(m_image);
        m_image.id = SG_INVALID_ID;
    }
    if (m_data) {
        m_data.reset();
    }
}

void
Image::make_image(size_t size, size_t width, size_t height)
{
    m_desc = (sg_image_desc){
        .label = m_label.c_str(),
        .width = static_cast<int>(width),
        .height = static_cast<int>(height),
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
        .data.subimage[0][0] = {
            .size = size,
            .ptr = *m_data,
        },
    };
    m_image = sg_make_image(&m_desc);
}

Image
Image::blank()
{
    uint8_t buf[4] = {255, 1, 255, 255};
    return Image("blank", &buf, 4, 1, 1);
}
