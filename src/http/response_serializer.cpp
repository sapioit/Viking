#include <http/response_serializer.h>
#include <http/components.h>
#include <misc/string_util.h>
#include <misc/dfa.h>
#include <misc/date.h>
#include <http/engine.h>
#include <string.h>

enum class States { StatusLine, GeneralHeader, ResponseHeader, Body, CRLFHeader, CRLFBody, End };
enum class Transitions { EndStatusLine, EndGeneralHeader, EndResponseHeader, EndBody, CRLFEnd, Error };

static constexpr auto start = States::StatusLine;
static constexpr auto end = States::End;
static constexpr auto crlf = "\r\n";
static constexpr auto crlfcrlf = "\r\n\r\n";
static std::string conn_close = "Close";
static std::string conn_keep_alive = "Keep-Alive";

DFA<States, Transitions> GetHttpResponseMachine() noexcept {
    static DFA<States, Transitions> machine(start, end);
    if (machine.StateNumber() == 0) {
        machine.Add(std::make_pair(States::StatusLine, Transitions::EndStatusLine), States::GeneralHeader);
        machine.Add(std::make_pair(States::GeneralHeader, Transitions::Error), States::CRLFBody);
        machine.Add(std::make_pair(States::GeneralHeader, Transitions::EndGeneralHeader), States::ResponseHeader);
        machine.Add(std::make_pair(States::ResponseHeader, Transitions::EndResponseHeader), States::CRLFHeader);
        machine.Add(std::make_pair(States::CRLFHeader, Transitions::CRLFEnd), States::Body);
        machine.Add(std::make_pair(States::Body, Transitions::EndBody), States::CRLFBody);
        machine.Add(std::make_pair(States::CRLFBody, Transitions::CRLFEnd), States::End);
    }
    return machine;
}

std::vector<char> ResponseSerializer::MakeHeader(const Http::Response &response) noexcept {
    // TODO rewrite this whole thing
    std::ostringstream stream;
    auto machine = GetHttpResponseMachine();

    while (machine.State() != States::Body) {
        switch (machine.State()) {
        case States::StatusLine: {
            stream << "HTTP/" << response.GetVersion().major << "." << response.GetVersion().minor;
            stream << " " << response.GetCode() << " ";
            stream << Http::StatusCodes.at(response.GetCode()) << crlf;
            machine.Transition(Transitions::EndStatusLine);
            break;
        }
        case States::GeneralHeader: {
            stream << "Date:"
                   << " " << StringUtil::ToString(Date::Now()) << crlf;
            stream << "Connection: ";
            if (response.GetKeepAlive())
                stream << conn_keep_alive;
            else
                stream << conn_close;
            stream << crlf;
            machine.Transition(Transitions::EndGeneralHeader);
            break;
        }
        case States::ResponseHeader: {
            auto content_type_str = Http::ContentTypes.at(response.GetContentType());
            stream << Http::Header::Fields::Content_Type << ": " << content_type_str << crlf;
            stream << Http::Header::Fields::Content_Length << ": " << response.ContentLength() << crlf;
            stream << Http::Header::Fields::Cache_Control << ": ";
            if (response.GetCachePolicy().max_age != 0)
                stream << "max-age=" + std::to_string(response.GetCachePolicy().max_age);
            else
                stream << "no-cache";
            stream << crlf;
            // TODO Research more into this
            stream << Http::Header::Fields::Transfer_Encoding << ": "
                   << "binary" << crlf;
            machine.Transition(Transitions::EndResponseHeader);
            break;
        }
        case States::CRLFHeader: {
            stream << crlf;
            machine.Transition(Transitions::CRLFEnd);
            break;
        }
        default: { break; }
        }
    }
    auto str = stream.str();
    return {str.begin(), str.end()};
}

std::vector<char> ResponseSerializer::MakeBody(const Http::Response &response) noexcept {
    switch (response.GetBodyType()) {
    case Http::Response::BodyType::Resource:
        return response.GetResource().content();
    case Http::Response::BodyType::Text:
        return {response.GetText().begin(), response.GetText().end()};
    default:
        return {};
    }
}

std::vector<char> ResponseSerializer::MakeEnding(const Http::Response &) noexcept {
    return std::vector<char>(crlfcrlf, crlfcrlf + strlen(crlfcrlf));
}

std::vector<char> ResponseSerializer::operator()(const Http::Response &response) noexcept {
    auto header = MakeHeader(response);
    auto body = MakeBody(response);
    auto ending = MakeEnding(response);

    std::vector<char> buffer;
    buffer.reserve(header.size() + body.size() + ending.size());
    buffer.insert(buffer.end(), header.begin(), header.end());
    buffer.insert(buffer.end(), body.begin(), body.end());
    buffer.insert(buffer.end(), ending.begin(), ending.end());
    return buffer;
}
