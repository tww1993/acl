#pragma once

#include <map>
#include <string>
#include <sstream>
#include <functional>

namespace acl {

class HttpServletRequest;
class HttpServletResponse;

typedef HttpServletRequest request_t;
typedef HttpServletResponse response_t;

typedef std::function<bool(request_t&, response_t&)> handler_t;

class http_servlet_impl : public HttpServlet {
public:
	http_servlet_impl(std::map<acl::string, handler_t>& handlers,
		socket_stream* stream, session* session)
	: HttpServlet(stream, session), handlers_(handlers) {}

	virtual ~http_servlet_impl(void) {}

protected:
	// override
	bool doGet(request_t& req, response_t& res) {
		return doPost(req, res);
	}

	// override
	bool doPost(request_t& req, response_t& res) {
		res.setKeepAlive(req.isKeepAlive());
		bool keep = req.isKeepAlive();

		const char* path = req.getPathInfo();
		if (path == NULL || *path == 0) {
			res.setStatus(400);
			acl::string buf("404 bad request\r\n");
			res.setContentLength(buf.size());
			return res.write(buf.c_str(), buf.size()) && keep;
		}

		size_t len = strlen(path);
		acl::string buf(path);
		if (path[len - 1] != '/') {
			buf += '/';
		}
		buf.lower();

		std::map<acl::string, handler_t>::iterator it
			= handlers_.find(buf);

		if (it != handlers_.end()) {
			return it->second(req, res) && keep;
		}

		res.setStatus(404);
		buf = "404 ";
		buf += path;
		buf += " not found\r\n";
		res.setContentLength(buf.size());
		return res.write(buf.c_str(), buf.size()) && keep;
	}

private:
	std::map<acl::string, handler_t> handlers_;
};

} // namespace acl
