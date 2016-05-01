/*
Copyright (C) 2015 Voinea Constantin Vladimir

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/
#include "ssl_socket.h"
#include <misc/common.h>
#include <misc/debug.h>
#include <mutex>

using namespace io;

static char certificate[] = "-----BEGIN CERTIFICATE-----\n"
                            "MIIDLjCCAhYCCQDL1lr6N8/gvzANBgkqhkiG9w0BAQUFADBZMQswCQYDVQQGEwJB\n"
                            "VTETMBEGA1UECBMKU29tZS1TdGF0ZTEhMB8GA1UEChMYSW50ZXJuZXQgV2lkZ2l0\n"
                            "cyBQdHkgTHRkMRIwEAYDVQQDEwlsb2NhbGhvc3QwHhcNMTQwNTEwMTcwODIzWhcN\n"
                            "MjQwNTA3MTcwODIzWjBZMQswCQYDVQQGEwJBVTETMBEGA1UECBMKU29tZS1TdGF0\n"
                            "ZTEhMB8GA1UEChMYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMRIwEAYDVQQDEwls\n"
                            "b2NhbGhvc3QwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDIltaUmHg+\n"
                            "G7Ida2XCtEQx1YeWDX41U2zBKbY0lT+auXf81cT3dYTdfJblb+v4CTWaGNofogcz\n"
                            "ebm8B2/OF9F+WWkKAJhKsTPAE7/SNAdi4Eqv4FfNbWKkGb4xacxxb4PH2XP9V3Ch\n"
                            "J6lMSI3V68FmEf4kcEN14V8vufIC5HE/LT4gCPDJ4UfUUbAgEhSebT6r/KFYB5T3\n"
                            "AeDc1VdnaaRblrP6KwM45vTs0Ii09/YrlzBxaTPMjLGCKa8JMv8PW2R0U9WCqHmz\n"
                            "BH+W3Q9xPrfhCInm4JWob8WgM1NuiYuzFB0CNaQcdMS7h0aZEAVnayhQ96/Padpj\n"
                            "KNE0Lur9nUxbAgMBAAEwDQYJKoZIhvcNAQEFBQADggEBAGRV71uRt/1dADsMD9fg\n"
                            "JvzW89jFAN87hXCRhTWxfXhYMzknxJ5WMb2JAlaMc/gTpiDiQBkbvB+iJe5AepgQ\n"
                            "WbyxPJNtSlA9GfKBz1INR5cFsOL27VrBoMYHMaolveeslc1AW2HfBtXWXeWSEF7F\n"
                            "QNgye8ZDPNzeSWSI0VyK2762wsTgTuUhHAaJ45660eX57+e8IvaM7xOEfBPDKYtU\n"
                            "0a28ZuhvSr2akJtGCwcs2J6rs6I+rV84UktDxFC9LUezBo8D9FkMPLoPKKNH1dXR\n"
                            "6LO8GOkqWUrhPIEmfy9KYes3q2ZX6svk4rwBtommHRv30kPxnnU1YXt52Ri+XczO\n"
                            "wEs=\n"
                            "-----END CERTIFICATE-----\n";

static char private_key[] = "-----BEGIN RSA PRIVATE KEY-----\n"
                            "MIIEpAIBAAKCAQEAyJbWlJh4PhuyHWtlwrREMdWHlg1+NVNswSm2NJU/mrl3/NXE\n"
                            "93WE3XyW5W/r+Ak1mhjaH6IHM3m5vAdvzhfRfllpCgCYSrEzwBO/0jQHYuBKr+BX\n"
                            "zW1ipBm+MWnMcW+Dx9lz/VdwoSepTEiN1evBZhH+JHBDdeFfL7nyAuRxPy0+IAjw\n"
                            "yeFH1FGwIBIUnm0+q/yhWAeU9wHg3NVXZ2mkW5az+isDOOb07NCItPf2K5cwcWkz\n"
                            "zIyxgimvCTL/D1tkdFPVgqh5swR/lt0PcT634QiJ5uCVqG/FoDNTbomLsxQdAjWk\n"
                            "HHTEu4dGmRAFZ2soUPevz2naYyjRNC7q/Z1MWwIDAQABAoIBAHrkryLrJwAmR8Hu\n"
                            "grH/b6h4glFUgvZ43jCaNZ+RsR5Cc1jcP4i832Izat+26oNUYRrADyNCSdcnxLuG\n"
                            "cuF5hkg6zzfplWRtnJ8ZenR2m+/gKuIGOMULN1wCyZvMjg0RnVNbzsxwPfj+K6Mo\n"
                            "8H0Xq621aFc60JnwMjkzWyqaeyeQogn1pqybuL6Dm2huvN49LR64uHuDUStTRX33\n"
                            "ou1fVWXOJ1kealYPbRPj8pDa31omB8q5Cf8Qe/b9anqyi9CsP17QbVg9k2IgoLlj\n"
                            "agqOc0u/opOTZB4tqJbqsIdEhc5LD5RUkYJsw00Iq0RSiKTfiWSPyOFw99Y9Act0\n"
                            "cbIIxEECgYEA8/SOsQjoUX1ipRvPbfO3suV1tU1hLCQbIpv7WpjNr1kHtngjzQMP\n"
                            "dU/iriUPGF1H+AxJJcJQfCVThV1AwFYVKb/LCrjaxlneZSbwfehpjo+xQGaNYG7Q\n"
                            "1vQuBVejuYk/IvpZltQOdm838DjvYyWDMh4dcMFIycXxEg+oHxf/s+8CgYEA0n4p\n"
                            "GBuLUNx9vv3e84BcarLaOF7wY7tb8z2oC/mXztMZpKjovTH0PvePgI5/b3KQ52R0\n"
                            "8zXHVX/4lSQVtCuhOVwKOCQq97/Zhlp5oTTShdQ0Qa1GQRl5wbTS6hrYEWSi9AQP\n"
                            "BVUPZ+RIcxx00DfBNURkId8xEpvCOmvySN8sUlUCgYAtXmHbEqkB3qulwRJGhHi5\n"
                            "UGsfmJBlwSE6wn9wTdKStZ/1k0o1KkiJrJ2ffUzdXxuvSbmgyA5nyBlMSBdurZOp\n"
                            "+/0qtU4abUQq058OC1b2KEryix/nuzQjha25WJ8eNiQDwUNABZfa9rwUdMIwUh2g\n"
                            "CHG5Mnjy7Vjz3u2JOtFXCQKBgQCVRo1EIHyLauLuaMINM9HWhWJGqeWXBM8v0GD1\n"
                            "pRsovQKpiHQNgHizkwM861GqqrfisZZSyKfFlcynkACoVmyu7fv9VoD2VCMiqdUq\n"
                            "IvjNmfE5RnXVQwja+668AS+MHi+GF77DTFBxoC5VHDAnXfLyIL9WWh9GEBoNLnKT\n"
                            "hVm8RQKBgQCB9Skzdftc+14a4Vj3NCgdHZHz9mcdPhzJXUiQyZ3tYhaytX9E8mWq\n"
                            "pm/OFqahbxw6EQd86mgANBMKayD6B1Id1INqtXN1XYI50bSs1D2nOGsBM7MK9aWD\n"
                            "JXlJ2hwsIc4q9En/LR3GtBaL84xTHGfznNylNhXi7GbO1wNMJuAukA==\n"
                            "-----END RSA PRIVATE KEY-----\n";

static char dhparams[] = "-----BEGIN DH PARAMETERS-----\n"
                         "MIIBCAKCAQEAy1+hVWCfNQoPB+NA733IVOONl8fCumiz9zdRRu1hzVa2yvGseUSq\n"
                         "Bbn6k0FQ7yMED6w5XWQKDC0z2m0FI/BPE3AjUfuPzEYGqTDf9zQZ2Lz4oAN90Sud\n"
                         "luOoEhYR99cEbCn0T4eBvEf9IUtczXUZ/wj7gzGbGG07dLfT+CmCRJxCjhrosenJ\n"
                         "gzucyS7jt1bobgU66JKkgMNm7hJY4/nhR5LWTCzZyzYQh2HM2Vk4K5ZqILpj/n0S\n"
                         "5JYTQ2PVhxP+Uu8+hICs/8VvM72DznjPZzufADipjC7CsQ4S6x/ecZluFtbb+ZTv\n"
                         "HI5CnYmkAwJ6+FSWGaZQDi8bgerFk9RWwwIBAg==\n"
                         "-----END DH PARAMETERS-----\n";

std::once_flag s2n_init_flag;

static s2n_config *get_config() {
    s2n_config *config = s2n_config_new();
    if (!config) {
        debug("config is null: " << s2n_strerror(s2n_errno, "EN"));
    }
    if (s2n_config_add_cert_chain_and_key(config, certificate, private_key) < 0) {
        debug("could not add certificate and private key: " << s2n_strerror(s2n_errno, "EN"));
    }
    if (s2n_config_add_dhparams(config, dhparams) < 0) {
        debug("could not add DH params: " << s2n_strerror(s2n_errno, "EN"));
    }
    return config;
}

ssl_socket::ssl_socket(int port) : tcp_socket(port), ssl_connection(nullptr), config(s2n_config_new()) {
    std::call_once(s2n_init_flag, s2n_init);
    flags |= 1 << 1;
}

ssl_socket::ssl_socket(int fd, int port) : tcp_socket(fd, port), ssl_connection(nullptr) {
    std::call_once(s2n_init_flag, s2n_init);
    flags |= 1 << 1;
    ssl_connection = s2n_connection_new(S2N_SERVER);
    if (!ssl_connection) {
        debug("SSL connection is null");
    }
    config = get_config();
    if (!config) {
        debug("Config is null");
    }
    s2n_connection_set_config(ssl_connection, config);

    if (s2n_connection_set_fd(ssl_connection, fd_) < 0) {
        debug("could not set ssl_connection fd: " << s2n_strerror(s2n_errno, "EN"));
    }

    s2n_blocked_status blocked;
    do {
        int ret = s2n_negotiate(ssl_connection, &blocked);
        if (ret < 0) {
            debug(s2n_errno);
            fprintf(stderr, "Failed to negotiate: '%s' %d\n", s2n_strerror(s2n_errno, "EN"),
                    s2n_connection_get_alert(ssl_connection));
            exit(1);
        }
    } while (blocked);
}

ssl_socket::ssl_socket(ssl_socket &&other) : tcp_socket(std::move(other)) {
    if (&other != this) {
        *this = std::move(other);
    }
}

ssl_socket &ssl_socket::operator=(ssl_socket &&other) {
    ssl_connection = other.ssl_connection;
    other.ssl_connection = nullptr;
    config = other.config;
    other.config = nullptr;
    return *this;
}

ssl_socket::~ssl_socket() {
    if (ssl_connection) {
        s2n_connection_wipe(ssl_connection);
    }
    if (config) {
        s2n_config_free(config);
    }
}

void print_data(s2n_connection *conn) {
    int client_hello_version;
    int client_protocol_version;
    int server_protocol_version;
    int actual_protocol_version;

    if ((client_hello_version = s2n_connection_get_client_hello_version(conn)) < 0) {
        fprintf(stderr, "Could not get client hello version\n");
        exit(1);
    }
    if ((client_protocol_version = s2n_connection_get_client_protocol_version(conn)) < 0) {
        fprintf(stderr, "Could not get client protocol version\n");
        exit(1);
    }
    if ((server_protocol_version = s2n_connection_get_server_protocol_version(conn)) < 0) {
        fprintf(stderr, "Could not get server protocol version\n");
        exit(1);
    }
    if ((actual_protocol_version = s2n_connection_get_actual_protocol_version(conn)) < 0) {
        fprintf(stderr, "Could not get actual protocol version\n");
        exit(1);
    }
    printf("Client hello version: %d\n", client_hello_version);
    printf("Client protocol version: %d\n", client_protocol_version);
    printf("Server protocol version: %d\n", server_protocol_version);
    printf("Actual protocol version: %d\n", actual_protocol_version);

    if (s2n_get_server_name(conn)) {
        printf("Server name: %s\n", s2n_get_server_name(conn));
    }
    if (s2n_get_application_protocol(conn)) {
        printf("Application protocol: %s\n", s2n_get_application_protocol(conn));
    }
    uint32_t length;
    const uint8_t *status = s2n_connection_get_ocsp_response(conn, &length);
    if (status && length > 0) {
        fprintf(stderr, "OCSP response received, length %d\n", length);
    }

    printf("Cipher negotiated: %s\n", s2n_connection_get_cipher(conn));
    fflush(stdout);
}

std::unique_ptr<tcp_socket> ssl_socket::accept() const {
    struct sockaddr in_addr;
    socklen_t in_len;
    in_len = sizeof(in_addr);

    int new_fd = ::accept(fd_, &in_addr, &in_len);
    if (unlikely(new_fd == -1)) {
        return nullptr;
    }
    tcp_socket *sock = new ssl_socket(new_fd, port_);
    return std::unique_ptr<tcp_socket>(sock);
}

std::size_t ssl_socket::write(const char *data, std::size_t len, tcp_socket::error_code &ec) const noexcept {
    ec = error_code::none;
    s2n_blocked_status blocked;
    auto total_to_write = len;
    std::size_t bytes_written_total = 0;
    ssize_t bytes_written_loop = 0;

    do {
        bytes_written_loop = s2n_send(ssl_connection, const_cast<char *>(data) + bytes_written_total,
                                      total_to_write - bytes_written_total, &blocked);
        if (bytes_written_loop <= 0) {
            break;
        }
        bytes_written_total += bytes_written_loop;
    } while (bytes_written_loop > 0 && bytes_written_total != total_to_write && blocked == S2N_NOT_BLOCKED);

    if (blocked == S2N_BLOCKED_ON_READ || blocked == S2N_BLOCKED_ON_WRITE)
        ec = error_code::blocked;

    if (bytes_written_loop == -1) {
        switch (errno) {
        case EWOULDBLOCK:
            ec = error_code::blocked;
            return bytes_written_total;
        case EPIPE:
        case ECONNRESET:
            ec = error_code::connection_closed_by_peer;
            break;
        default:
            break;
        }
    }
    return bytes_written_total;
}

std::size_t ssl_socket::read(char *const data, std::size_t len, tcp_socket::error_code &ec) const noexcept {
    ec = error_code::none;
    s2n_blocked_status blocked;
    std::size_t bytes_read_total = 0;
    ssize_t bytes_read_loop = 0;
    do {
        if (bytes_read_loop > 0)
            bytes_read_total += bytes_read_loop;
        bytes_read_loop =
            s2n_recv(ssl_connection, const_cast<char *>(data) + bytes_read_total, len - bytes_read_total, &blocked);

    } while (bytes_read_total != len && blocked == S2N_NOT_BLOCKED && bytes_read_loop > 0);

    if (blocked == S2N_BLOCKED_ON_READ || blocked == S2N_BLOCKED_ON_WRITE)
        ec = error_code::blocked;
    if (bytes_read_loop == 0)
        ec = error_code::connection_closed_by_peer;
    return bytes_read_total;
}

std::string ssl_socket::read(tcp_socket::error_code &ec) const noexcept {
    ec = error_code::none;
    static auto page_size = getpagesize();
    std::string vec;
    std::size_t readloop = 0;
    do {
        std::string tmp;
        tmp.resize(page_size);
        readloop += this->read(&tmp.front(), page_size, ec);
        tmp.resize(readloop);
        vec += std::move(tmp);
    } while (ec == error_code::none);
    return vec;
}
