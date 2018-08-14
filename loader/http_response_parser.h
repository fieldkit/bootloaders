#ifndef FK_HTTP_RESPONSE_PARSER_H_INCLUDED
#define FK_HTTP_RESPONSE_PARSER_H_INCLUDED

#include <cinttypes>
#include <cstdlib>

namespace fk {

class HttpResponseParser {
private:
    static constexpr size_t BufferSize = 64;
    static constexpr size_t MaximumETagSize = BufferSize;

    bool reading_header_{ true };
    uint8_t consecutive_nls_{ 0 };
    uint8_t previous_{ 0 };
    uint8_t spaces_seen_{ 0 };
    uint8_t position_{ 0 };
    uint16_t status_code_{ 0 };
    uint32_t content_length_{ 0 };
    char buffer_[BufferSize];
    char etag_[MaximumETagSize];

public:
    bool reading_header() {
        return reading_header_;
    }

    uint16_t status_code() {
        return status_code_;
    }

    uint32_t content_length() {
        return content_length_;
    }

    const char *etag() {
        return etag_;
    }

public:
    void begin();

    void write(uint8_t c);

};

}

#endif
