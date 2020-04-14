//
//  httplib.h
//
//  Copyright (c) 2020 Yuji Hirose. All rights reserved.
//  MIT License
//

#ifndef CPPHTTPLIB_HTTPLIB_H
#define CPPHTTPLIB_HTTPLIB_H

/*
 * Configuration
 */

#ifndef CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND
#define CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND 5
#endif

#ifndef CPPHTTPLIB_KEEPALIVE_TIMEOUT_USECOND
#define CPPHTTPLIB_KEEPALIVE_TIMEOUT_USECOND 0
#endif

#ifndef CPPHTTPLIB_KEEPALIVE_MAX_COUNT
#define CPPHTTPLIB_KEEPALIVE_MAX_COUNT 5
#endif

#ifndef CPPHTTPLIB_READ_TIMEOUT_SECOND
#define CPPHTTPLIB_READ_TIMEOUT_SECOND 5
#endif

#ifndef CPPHTTPLIB_READ_TIMEOUT_USECOND
#define CPPHTTPLIB_READ_TIMEOUT_USECOND 0
#endif

#ifndef CPPHTTPLIB_REQUEST_URI_MAX_LENGTH
#define CPPHTTPLIB_REQUEST_URI_MAX_LENGTH 8192
#endif

#ifndef CPPHTTPLIB_REDIRECT_MAX_COUNT
#define CPPHTTPLIB_REDIRECT_MAX_COUNT 20
#endif

#ifndef CPPHTTPLIB_PAYLOAD_MAX_LENGTH
#define CPPHTTPLIB_PAYLOAD_MAX_LENGTH ((std::numeric_limits<size_t>::max)())
#endif

#ifndef CPPHTTPLIB_RECV_BUFSIZ
#define CPPHTTPLIB_RECV_BUFSIZ size_t(4096u)
#endif

#ifndef CPPHTTPLIB_THREAD_POOL_COUNT
#define CPPHTTPLIB_THREAD_POOL_COUNT                                           \
  ((std::max)(1u, std::thread::hardware_concurrency() - 1))
#endif

/*
 * Headers
 */

#ifdef _WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif //_CRT_SECURE_NO_WARNINGS

#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif //_CRT_NONSTDC_NO_DEPRECATE

#if defined(_MSC_VER)
#ifdef _WIN64
using ssize_t = __int64;
#else
using ssize_t = int;
#endif

#if _MSC_VER < 1900
#define snprintf _snprintf_s
#endif
#endif // _MSC_VER

#ifndef S_ISREG
#define S_ISREG(m) (((m)&S_IFREG) == S_IFREG)
#endif // S_ISREG

#ifndef S_ISDIR
#define S_ISDIR(m) (((m)&S_IFDIR) == S_IFDIR)
#endif // S_ISDIR

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#ifndef WSA_FLAG_NO_HANDLE_INHERIT
#define WSA_FLAG_NO_HANDLE_INHERIT 0x80
#endif

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

#ifndef strcasecmp
#define strcasecmp _stricmp
#endif // strcasecmp

using socket_t = SOCKET;
#ifdef CPPHTTPLIB_USE_POLL
#define poll(fds, nfds, timeout) WSAPoll(fds, nfds, timeout)
#endif

#else // not _WIN32

#include <arpa/inet.h>
#include <cstring>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#ifdef CPPHTTPLIB_USE_POLL
#include <poll.h>
#endif
#include <csignal>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

using socket_t = int;
#define INVALID_SOCKET (-1)
#endif //_WIN32

#include <array>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <regex>
#include <string>
#include <sys/stat.h>
#include <thread>

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
#include <openssl/err.h>
#include <openssl/md5.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#include <iomanip>
#include <iostream>
#include <sstream>

// #if OPENSSL_VERSION_NUMBER < 0x1010100fL
// #error Sorry, OpenSSL versions prior to 1.1.1 are not supported
// #endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#include <openssl/crypto.h>
inline const unsigned char *ASN1_STRING_get0_data(const ASN1_STRING *asn1) {
  return M_ASN1_STRING_data(asn1);
}
#endif
#endif

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
#include <zlib.h>
#endif

/*
 * Declaration
 */
namespace httplib {

namespace detail {

struct ci {
  bool operator()(const std::string &s1, const std::string &s2) const {
    return std::lexicographical_compare(
        s1.begin(), s1.end(), s2.begin(), s2.end(),
        [](char c1, char c2) { return ::tolower(c1) < ::tolower(c2); });
  }
};

} // namespace detail

using Headers = std::multimap<std::string, std::string, detail::ci>;

using Params = std::multimap<std::string, std::string>;
using Match = std::smatch;

using Progress = std::function<bool(uint64_t current, uint64_t total)>;

struct Response;
using ResponseHandler = std::function<bool(const Response &response)>;

struct MultipartFormData {
  std::string name;
  std::string content;
  std::string filename;
  std::string content_type;
};
using MultipartFormDataItems = std::vector<MultipartFormData>;
using MultipartFormDataMap = std::multimap<std::string, MultipartFormData>;

class DataSink {
public:
  DataSink() = default;
  DataSink(const DataSink &) = delete;
  DataSink &operator=(const DataSink &) = delete;
  DataSink(DataSink &&) = delete;
  DataSink &operator=(DataSink &&) = delete;

  std::function<void(const char *data, size_t data_len)> write;
  std::function<void()> done;
  std::function<bool()> is_writable;
};

using ContentProvider =
    std::function<void(size_t offset, size_t length, DataSink &sink)>;

using ContentReceiver =
    std::function<bool(const char *data, size_t data_length)>;

using MultipartContentHeader =
    std::function<bool(const MultipartFormData &file)>;

class ContentReader {
public:
  using Reader = std::function<bool(ContentReceiver receiver)>;
  using MultipartReader = std::function<bool(MultipartContentHeader header,
                                             ContentReceiver receiver)>;

  ContentReader(Reader reader, MultipartReader muitlpart_reader)
      : reader_(reader), muitlpart_reader_(muitlpart_reader) {}

  bool operator()(MultipartContentHeader header,
                  ContentReceiver receiver) const {
    return muitlpart_reader_(header, receiver);
  }

  bool operator()(ContentReceiver receiver) const { return reader_(receiver); }

  Reader reader_;
  MultipartReader muitlpart_reader_;
};

using Range = std::pair<ssize_t, ssize_t>;
using Ranges = std::vector<Range>;

struct Request {
  std::string method;
  std::string path;
  Headers headers;
  std::string body;

  // for server
  std::string version;
  std::string target;
  Params params;
  MultipartFormDataMap files;
  Ranges ranges;
  Match matches;

  // for client
  size_t redirect_count = CPPHTTPLIB_REDIRECT_MAX_COUNT;
  ResponseHandler response_handler;
  ContentReceiver content_receiver;
  Progress progress;

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  const SSL *ssl;
#endif

  bool has_header(const char *key) const;
  std::string get_header_value(const char *key, size_t id = 0) const;
  size_t get_header_value_count(const char *key) const;
  void set_header(const char *key, const char *val);
  void set_header(const char *key, const std::string &val);

  bool has_param(const char *key) const;
  std::string get_param_value(const char *key, size_t id = 0) const;
  size_t get_param_value_count(const char *key) const;

  bool is_multipart_form_data() const;

  bool has_file(const char *key) const;
  MultipartFormData get_file_value(const char *key) const;

  // private members...
  size_t content_length;
  ContentProvider content_provider;
};

struct Response {
  std::string version;
  int status = -1;
  Headers headers;
  std::string body;

  bool has_header(const char *key) const;
  std::string get_header_value(const char *key, size_t id = 0) const;
  size_t get_header_value_count(const char *key) const;
  void set_header(const char *key, const char *val);
  void set_header(const char *key, const std::string &val);

  void set_redirect(const char *url);
  void set_content(const char *s, size_t n, const char *content_type);
  void set_content(std::string s, const char *content_type);

  void set_content_provider(
      size_t length,
      std::function<void(size_t offset, size_t length, DataSink &sink)>
          provider,
      std::function<void()> resource_releaser = [] {});

  void set_chunked_content_provider(
      std::function<void(size_t offset, DataSink &sink)> provider,
      std::function<void()> resource_releaser = [] {});

  Response() = default;
  Response(const Response &) = default;
  Response &operator=(const Response &) = default;
  Response(Response &&) = default;
  Response &operator=(Response &&) = default;
  ~Response() {
    if (content_provider_resource_releaser) {
      content_provider_resource_releaser();
    }
  }

  // private members...
  size_t content_length = 0;
  ContentProvider content_provider;
  std::function<void()> content_provider_resource_releaser;
};

class Stream {
public:
  virtual ~Stream() = default;

  virtual bool is_readable() const = 0;
  virtual bool is_writable() const = 0;

  virtual ssize_t read(char *ptr, size_t size) = 0;
  virtual ssize_t write(const char *ptr, size_t size) = 0;
  virtual std::string get_remote_addr() const = 0;

  template <typename... Args>
  ssize_t write_format(const char *fmt, const Args &... args);
  ssize_t write(const char *ptr);
  ssize_t write(const std::string &s);
};

class TaskQueue {
public:
  TaskQueue() = default;
  virtual ~TaskQueue() = default;
  virtual void enqueue(std::function<void()> fn) = 0;
  virtual void shutdown() = 0;
};

class ThreadPool : public TaskQueue {
public:
  explicit ThreadPool(size_t n) : shutdown_(false) {
    while (n) {
      threads_.emplace_back(worker(*this));
      n--;
    }
  }

  ThreadPool(const ThreadPool &) = delete;
  ~ThreadPool() override = default;

  void enqueue(std::function<void()> fn) override {
    std::unique_lock<std::mutex> lock(mutex_);
    jobs_.push_back(fn);
    cond_.notify_one();
  }

  void shutdown() override {
    // Stop all worker threads...
    {
      std::unique_lock<std::mutex> lock(mutex_);
      shutdown_ = true;
    }

    cond_.notify_all();

    // Join...
    for (auto &t : threads_) {
      t.join();
    }
  }

private:
  struct worker {
    explicit worker(ThreadPool &pool) : pool_(pool) {}

    void operator()() {
      for (;;) {
        std::function<void()> fn;
        {
          std::unique_lock<std::mutex> lock(pool_.mutex_);

          pool_.cond_.wait(
              lock, [&] { return !pool_.jobs_.empty() || pool_.shutdown_; });

          if (pool_.shutdown_ && pool_.jobs_.empty()) { break; }

          fn = pool_.jobs_.front();
          pool_.jobs_.pop_front();
        }

        assert(true == static_cast<bool>(fn));
        fn();
      }
    }

    ThreadPool &pool_;
  };
  friend struct worker;

  std::vector<std::thread> threads_;
  std::list<std::function<void()>> jobs_;

  bool shutdown_;

  std::condition_variable cond_;
  std::mutex mutex_;
};

using Logger = std::function<void(const Request &, const Response &)>;

class Server {
public:
  using Handler = std::function<void(const Request &, Response &)>;
  using HandlerWithContentReader = std::function<void(
      const Request &, Response &, const ContentReader &content_reader)>;
  using Expect100ContinueHandler =
      std::function<int(const Request &, Response &)>;

  Server();

  virtual ~Server();

  virtual bool is_valid() const;

  Server &Get(const char *pattern, Handler handler);
  Server &Post(const char *pattern, Handler handler);
  Server &Post(const char *pattern, HandlerWithContentReader handler);
  Server &Put(const char *pattern, Handler handler);
  Server &Put(const char *pattern, HandlerWithContentReader handler);
  Server &Patch(const char *pattern, Handler handler);
  Server &Patch(const char *pattern, HandlerWithContentReader handler);
  Server &Delete(const char *pattern, Handler handler);
  Server &Delete(const char *pattern, HandlerWithContentReader handler);
  Server &Options(const char *pattern, Handler handler);

  [[deprecated]] bool set_base_dir(const char *dir,
                                   const char *mount_point = nullptr);
  bool set_mount_point(const char *mount_point, const char *dir);
  bool remove_mount_point(const char *mount_point);
  void set_file_extension_and_mimetype_mapping(const char *ext,
                                               const char *mime);
  void set_file_request_handler(Handler handler);

  void set_error_handler(Handler handler);
  void set_logger(Logger logger);

  void set_expect_100_continue_handler(Expect100ContinueHandler handler);

  void set_keep_alive_max_count(size_t count);
  void set_read_timeout(time_t sec, time_t usec);
  void set_payload_max_length(size_t length);

  bool bind_to_port(const char *host, int port, int socket_flags = 0);
  int bind_to_any_port(const char *host, int socket_flags = 0);
  bool listen_after_bind();

  bool listen(const char *host, int port, int socket_flags = 0);

  bool is_running() const;
  void stop();

  std::function<TaskQueue *(void)> new_task_queue;

protected:
  bool process_request(Stream &strm, bool last_connection,
                       bool &connection_close,
                       const std::function<void(Request &)> &setup_request);

  size_t keep_alive_max_count_;
  time_t read_timeout_sec_;
  time_t read_timeout_usec_;
  size_t payload_max_length_;

private:
  using Handlers = std::vector<std::pair<std::regex, Handler>>;
  using HandlersForContentReader =
      std::vector<std::pair<std::regex, HandlerWithContentReader>>;

  socket_t create_server_socket(const char *host, int port,
                                int socket_flags) const;
  int bind_internal(const char *host, int port, int socket_flags);
  bool listen_internal();

  bool routing(Request &req, Response &res, Stream &strm);
  bool handle_file_request(Request &req, Response &res, bool head = false);
  bool dispatch_request(Request &req, Response &res, Handlers &handlers);
  bool dispatch_request_for_content_reader(Request &req, Response &res,
                                           ContentReader content_reader,
                                           HandlersForContentReader &handlers);

  bool parse_request_line(const char *s, Request &req);
  bool write_response(Stream &strm, bool last_connection, const Request &req,
                      Response &res);
  bool write_content_with_provider(Stream &strm, const Request &req,
                                   Response &res, const std::string &boundary,
                                   const std::string &content_type);
  bool read_content(Stream &strm, Request &req, Response &res);
  bool
  read_content_with_content_receiver(Stream &strm, Request &req, Response &res,
                                     ContentReceiver receiver,
                                     MultipartContentHeader multipart_header,
                                     ContentReceiver multipart_receiver);
  bool read_content_core(Stream &strm, Request &req, Response &res,
                         ContentReceiver receiver,
                         MultipartContentHeader mulitpart_header,
                         ContentReceiver multipart_receiver);

  virtual bool process_and_close_socket(socket_t sock);

  std::atomic<bool> is_running_;
  std::atomic<socket_t> svr_sock_;
  std::vector<std::pair<std::string, std::string>> base_dirs_;
  std::map<std::string, std::string> file_extension_and_mimetype_map_;
  Handler file_request_handler_;
  Handlers get_handlers_;
  Handlers post_handlers_;
  HandlersForContentReader post_handlers_for_content_reader_;
  Handlers put_handlers_;
  HandlersForContentReader put_handlers_for_content_reader_;
  Handlers patch_handlers_;
  HandlersForContentReader patch_handlers_for_content_reader_;
  Handlers delete_handlers_;
  HandlersForContentReader delete_handlers_for_content_reader_;
  Handlers options_handlers_;
  Handler error_handler_;
  Logger logger_;
  Expect100ContinueHandler expect_100_continue_handler_;
};

class Client {
public:
  explicit Client(const std::string &host, int port = 80,
                  const std::string &client_cert_path = std::string(),
                  const std::string &client_key_path = std::string());

  virtual ~Client();

  virtual bool is_valid() const;

  std::shared_ptr<Response> Get(const char *path);

  std::shared_ptr<Response> Get(const char *path, const Headers &headers);

  std::shared_ptr<Response> Get(const char *path, Progress progress);

  std::shared_ptr<Response> Get(const char *path, const Headers &headers,
                                Progress progress);

  std::shared_ptr<Response> Get(const char *path,
                                ContentReceiver content_receiver);

  std::shared_ptr<Response> Get(const char *path, const Headers &headers,
                                ContentReceiver content_receiver);

  std::shared_ptr<Response>
  Get(const char *path, ContentReceiver content_receiver, Progress progress);

  std::shared_ptr<Response> Get(const char *path, const Headers &headers,
                                ContentReceiver content_receiver,
                                Progress progress);

  std::shared_ptr<Response> Get(const char *path, const Headers &headers,
                                ResponseHandler response_handler,
                                ContentReceiver content_receiver);

  std::shared_ptr<Response> Get(const char *path, const Headers &headers,
                                ResponseHandler response_handler,
                                ContentReceiver content_receiver,
                                Progress progress);

  std::shared_ptr<Response> Head(const char *path);

  std::shared_ptr<Response> Head(const char *path, const Headers &headers);

  std::shared_ptr<Response> Post(const char *path, const std::string &body,
                                 const char *content_type);

  std::shared_ptr<Response> Post(const char *path, const Headers &headers,
                                 const std::string &body,
                                 const char *content_type);

  std::shared_ptr<Response> Post(const char *path, size_t content_length,
                                 ContentProvider content_provider,
                                 const char *content_type);

  std::shared_ptr<Response> Post(const char *path, const Headers &headers,
                                 size_t content_length,
                                 ContentProvider content_provider,
                                 const char *content_type);

  std::shared_ptr<Response> Post(const char *path, const Params &params);

  std::shared_ptr<Response> Post(const char *path, const Headers &headers,
                                 const Params &params);

  std::shared_ptr<Response> Post(const char *path,
                                 const MultipartFormDataItems &items);

  std::shared_ptr<Response> Post(const char *path, const Headers &headers,
                                 const MultipartFormDataItems &items);

  std::shared_ptr<Response> Put(const char *path, const std::string &body,
                                const char *content_type);

  std::shared_ptr<Response> Put(const char *path, const Headers &headers,
                                const std::string &body,
                                const char *content_type);

  std::shared_ptr<Response> Put(const char *path, size_t content_length,
                                ContentProvider content_provider,
                                const char *content_type);

  std::shared_ptr<Response> Put(const char *path, const Headers &headers,
                                size_t content_length,
                                ContentProvider content_provider,
                                const char *content_type);

  std::shared_ptr<Response> Put(const char *path, const Params &params);

  std::shared_ptr<Response> Put(const char *path, const Headers &headers,
                                const Params &params);

  std::shared_ptr<Response> Patch(const char *path, const std::string &body,
                                  const char *content_type);

  std::shared_ptr<Response> Patch(const char *path, const Headers &headers,
                                  const std::string &body,
                                  const char *content_type);

  std::shared_ptr<Response> Patch(const char *path, size_t content_length,
                                  ContentProvider content_provider,
                                  const char *content_type);

  std::shared_ptr<Response> Patch(const char *path, const Headers &headers,
                                  size_t content_length,
                                  ContentProvider content_provider,
                                  const char *content_type);

  std::shared_ptr<Response> Delete(const char *path);

  std::shared_ptr<Response> Delete(const char *path, const std::string &body,
                                   const char *content_type);

  std::shared_ptr<Response> Delete(const char *path, const Headers &headers);

  std::shared_ptr<Response> Delete(const char *path, const Headers &headers,
                                   const std::string &body,
                                   const char *content_type);

  std::shared_ptr<Response> Options(const char *path);

  std::shared_ptr<Response> Options(const char *path, const Headers &headers);

  bool send(const Request &req, Response &res);

  bool send(const std::vector<Request> &requests,
            std::vector<Response> &responses);

  void set_timeout_sec(time_t timeout_sec);

  void set_read_timeout(time_t sec, time_t usec);

  void set_keep_alive_max_count(size_t count);

  void set_basic_auth(const char *username, const char *password);

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  void set_digest_auth(const char *username, const char *password);
#endif

  void set_follow_location(bool on);

  void set_compress(bool on);

  void set_interface(const char *intf);

  void set_proxy(const char *host, int port);

  void set_proxy_basic_auth(const char *username, const char *password);

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  void set_proxy_digest_auth(const char *username, const char *password);
#endif

  void set_logger(Logger logger);

protected:
  bool process_request(Stream &strm, const Request &req, Response &res,
                       bool last_connection, bool &connection_close);

  const std::string host_;
  const int port_;
  const std::string host_and_port_;

  // Settings
  std::string client_cert_path_;
  std::string client_key_path_;

  time_t timeout_sec_ = 300;
  time_t read_timeout_sec_ = CPPHTTPLIB_READ_TIMEOUT_SECOND;
  time_t read_timeout_usec_ = CPPHTTPLIB_READ_TIMEOUT_USECOND;

  size_t keep_alive_max_count_ = CPPHTTPLIB_KEEPALIVE_MAX_COUNT;

  std::string basic_auth_username_;
  std::string basic_auth_password_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  std::string digest_auth_username_;
  std::string digest_auth_password_;
#endif

  bool follow_location_ = false;

  bool compress_ = false;

  std::string interface_;

  std::string proxy_host_;
  int proxy_port_;

  std::string proxy_basic_auth_username_;
  std::string proxy_basic_auth_password_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  std::string proxy_digest_auth_username_;
  std::string proxy_digest_auth_password_;
#endif

  Logger logger_;

  void copy_settings(const Client &rhs) {
    client_cert_path_ = rhs.client_cert_path_;
    client_key_path_ = rhs.client_key_path_;
    timeout_sec_ = rhs.timeout_sec_;
    read_timeout_sec_ = rhs.read_timeout_sec_;
    read_timeout_usec_ = rhs.read_timeout_usec_;
    keep_alive_max_count_ = rhs.keep_alive_max_count_;
    basic_auth_username_ = rhs.basic_auth_username_;
    basic_auth_password_ = rhs.basic_auth_password_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    digest_auth_username_ = rhs.digest_auth_username_;
    digest_auth_password_ = rhs.digest_auth_password_;
#endif
    follow_location_ = rhs.follow_location_;
    compress_ = rhs.compress_;
    interface_ = rhs.interface_;
    proxy_host_ = rhs.proxy_host_;
    proxy_port_ = rhs.proxy_port_;
    proxy_basic_auth_username_ = rhs.proxy_basic_auth_username_;
    proxy_basic_auth_password_ = rhs.proxy_basic_auth_password_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    proxy_digest_auth_username_ = rhs.proxy_digest_auth_username_;
    proxy_digest_auth_password_ = rhs.proxy_digest_auth_password_;
#endif
    logger_ = rhs.logger_;
  }

private:
  socket_t create_client_socket() const;
  bool read_response_line(Stream &strm, Response &res);
  bool write_request(Stream &strm, const Request &req, bool last_connection);
  bool redirect(const Request &req, Response &res);
  bool handle_request(Stream &strm, const Request &req, Response &res,
                      bool last_connection, bool &connection_close);
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  bool connect(socket_t sock, Response &res, bool &error);
#endif

  std::shared_ptr<Response> send_with_content_provider(
      const char *method, const char *path, const Headers &headers,
      const std::string &body, size_t content_length,
      ContentProvider content_provider, const char *content_type);

  virtual bool process_and_close_socket(
      socket_t sock, size_t request_count,
      std::function<bool(Stream &strm, bool last_connection,
                         bool &connection_close)>
          callback);

  virtual bool is_ssl() const;
};

inline void Get(std::vector<Request> &requests, const char *path,
                const Headers &headers) {
  Request req;
  req.method = "GET";
  req.path = path;
  req.headers = headers;
  requests.emplace_back(std::move(req));
}

inline void Get(std::vector<Request> &requests, const char *path) {
  Get(requests, path, Headers());
}

inline void Post(std::vector<Request> &requests, const char *path,
                 const Headers &headers, const std::string &body,
                 const char *content_type) {
  Request req;
  req.method = "POST";
  req.path = path;
  req.headers = headers;
  req.headers.emplace("Content-Type", content_type);
  req.body = body;
  requests.emplace_back(std::move(req));
}

inline void Post(std::vector<Request> &requests, const char *path,
                 const std::string &body, const char *content_type) {
  Post(requests, path, Headers(), body, content_type);
}

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
class SSLServer : public Server {
public:
  SSLServer(const char *cert_path, const char *private_key_path,
            const char *client_ca_cert_file_path = nullptr,
            const char *client_ca_cert_dir_path = nullptr);

  ~SSLServer() override;

  bool is_valid() const override;

private:
  bool process_and_close_socket(socket_t sock) override;

  SSL_CTX *ctx_;
  std::mutex ctx_mutex_;
};

class SSLClient : public Client {
public:
  explicit SSLClient(const std::string &host, int port = 443,
                     const std::string &client_cert_path = std::string(),
                     const std::string &client_key_path = std::string());

  ~SSLClient() override;

  bool is_valid() const override;

  void set_ca_cert_path(const char *ca_ceert_file_path,
                        const char *ca_cert_dir_path = nullptr);

  void enable_server_certificate_verification(bool enabled);

  long get_openssl_verify_result() const;

  SSL_CTX *ssl_context() const noexcept;

private:
  bool process_and_close_socket(
      socket_t sock, size_t request_count,
      std::function<bool(Stream &strm, bool last_connection,
                         bool &connection_close)>
          callback) override;
  bool is_ssl() const override;

  bool verify_host(X509 *server_cert) const;
  bool verify_host_with_subject_alt_name(X509 *server_cert) const;
  bool verify_host_with_common_name(X509 *server_cert) const;
  bool check_host_name(const char *pattern, size_t pattern_len) const;

  SSL_CTX *ctx_;
  std::mutex ctx_mutex_;
  std::vector<std::string> host_components_;

  std::string ca_cert_file_path_;
  std::string ca_cert_dir_path_;
  bool server_certificate_verification_ = false;
  long verify_result_ = 0;
};
#endif


} // namespace httplib

#endif // CPPHTTPLIB_HTTPLIB_H
