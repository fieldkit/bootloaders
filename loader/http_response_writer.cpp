#include <cstdlib>

#include "http_response_writer.h"

namespace fk {

HttpHeadersWriter::HttpHeadersWriter(Print &stream) : stream(stream) {
}

void HttpHeadersWriter::writeHeaders(Url &url, const char *method, OutgoingHttpHeaders &headers) {
    stream.print(method);
    stream.print(" /");
    stream.print(url.path);
    stream.println(" HTTP/1.1");

    stream.print("Host: ");
    stream.println(url.server);

    stream.println("Connection: close");

    if (headers.contentLength != OutgoingHttpHeaders::InvalidContentLength) {
        stream.print("Content-Length: ");
        stream.println(headers.contentLength);
    }

    if (headers.contentType != nullptr) {
        stream.print("Content-Type: ");
        stream.println(headers.contentType);
    }

    if (headers.etag != nullptr) {
        auto len = strlen(headers.etag);
        auto quote = headers.etag[0] != '"' && headers.etag[len - 1] != '"';
        stream.print("If-None-Match: ");
        if (quote) {
            stream.print('"');
        }
        stream.print(headers.etag);
        if (quote) {
            stream.print('"');
        }
        stream.println();
    }

    if (headers.fileId != OutgoingHttpHeaders::InvalidFileId) {
        stream.print("Fk-FileId: ");
        stream.println(headers.fileId);
    }

    if (headers.version != nullptr) {
        stream.print("Fk-Version: ");
        stream.println(headers.version);
    }

    if (headers.build != nullptr) {
        stream.print("Fk-Build: ");
        stream.println(headers.build);
    }

    if (headers.deviceId != nullptr) {
        stream.print("Fk-DeviceId: ");
        stream.println(headers.deviceId);
    }

    stream.println();
}

}
