#pragma once
#include <memory>
#include <filesystem>
#include <sokol_gfx.h>
#include <fmt/core.h>
#include <glm/gtc/matrix_transform.hpp>

class Image {
public:
    Image() {};
    Image(const std::filesystem::path &);
    Image(const char *p) : Image(std::filesystem::path{p}) {};
    Image(const std::string &label,
            const void *data,
            size_t size,
            size_t width,
            size_t height);
    Image(Image&&);
    ~Image();

    Image& operator=(Image&&);

    static Image blank();

    // free all resources for the image, and reset to empty state
    void reset();

    inline const std::string& label() const { return m_label; }
    inline int width() const { return m_desc.width; }
    inline int height() const { return m_desc.height; }
    inline size_t size() const { return m_desc.data.subimage[0][0].size; }
    inline const sg_image sg_image() const { return m_image; }
    inline bool valid() const { return m_data != nullptr; }
    inline const void * data() const { return m_data.get(); }

    // return a 3x3 matrix to convert pixel  (x, y) cords within the cropped
    // texture region to normalized (x, y) coords within the whole texture.
    inline glm::mat3 const crop_transform(int x, int y, int w, int h) {
        float width  = static_cast<float>(m_desc.width);
        float height = static_cast<float>(m_desc.height);
        return {
            { w/width, 0,                  0 },    // column 0
            { 0,                h/height, 0 },    // column 1
            { x/width,     y/height,      1 }     // column 2
        };
    }

private:
    void make_image(size_t size, size_t width, size_t height);

    std::string m_label{"no_label"};
    std::unique_ptr<unsigned char *> m_data{};
    sg_image_desc m_desc{};
    struct sg_image m_image{};
};

template <>
struct fmt::formatter<Image> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(const Image& obj, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(),
                "\"{}\" (tid {}, {}x{}, {} bytes @ {})",
                obj.label(),
                obj.sg_image().id,
                obj.width(),
                obj.height(),
                obj.size(),
                obj.data());
    }
};
