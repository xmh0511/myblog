#include "xmart.hpp"
using namespace xmart;

#include "./controller/index.hpp"

struct session_init {
	bool before(request& req, response& res) {
		auto& session = req.session("XMART_BLOG");
		//std::cout << "init session" << std::endl;
		if (session.empty()) {
			auto& session0 = req.create_session("XMART_BLOG");
			session0.set_expires(3600);
		}
		return true;
	}
	bool after(request& req, response& res) {
		return true;
	}
};

struct base_path_aop {
	bool before(request& req, response& res) {
		std::stringstream ss;
		std::ifstream file("./www.config");
		if (!file.is_open()) {
			return false;
		}
		ss << file.rdbuf();
		auto json = json::parse(ss.str());
		auto base_path = json["base_path"].get<std::string>();
		res.set_attr("base_path", base_path);
		auto is_proxy = json["proxy"].get<bool>();
		res.set_attr("proxy", is_proxy);
		return true;
	}
	bool after(request& req, response& res) {
		return true;
	}
};

struct check_login {
	bool before(request& req, response& res) {
		auto& session = req.session("XMART_BLOG");
		//std::cout << "session is empty: " << session.empty() << std::endl;
		if (session.get_data<std::string>("islogin") != "true") {
			res.write_view("./www/login.html");
			return false;
		}
		return true;
	}
	bool after(request& req, response& res) {
		return true;
	}
};

struct check_login_ajax {
	bool before(request& req, response& res) {
		auto& session = req.session("XMART_BLOG");
		if (session.get_data<std::string>("islogin") != "true") {
			json message;
			message["success"] = false;
			message["message"] = "用户认证失败";
			res.write_json(message);
			return false;
		}
		return true;
	}
	bool after(request& req, response& res) {
		return true;
	}
};

int main() {
	bool r = false;
	http_server& server = init_xmart("./config.json", r);
	if (!r) {
		std::cout<<"config has some error"<<std::endl;
		return 0;
	}

	server.set_url_redirect(false);

	server.add_view_method("calcNumber", 2, [](inja::Arguments& args)->json {
		auto src = args.at(0)->get<int>();
		auto number = args.at(1)->get<int>();
		auto r = src + number;
		return r;
	});

	server.router<GET>("/", &Index::index, nullptr, session_init{}, base_path_aop{});

	server.router<POST,GET>("/login", &Index::login, nullptr, session_init{}, base_path_aop{});

	server.router<GET>("/myblog", &Index::blog, nullptr, session_init{}, base_path_aop{}, check_login{});

	server.router<GET>("/addblog", &Index::addblog, nullptr, session_init{}, base_path_aop{}, check_login{});

	server.router<POST>("/addarticle", &Index::addarticle, nullptr, session_init{}, base_path_aop{}, check_login_ajax{});

	server.router<GET>("/detail/*", &Index::detail, nullptr, session_init{}, base_path_aop{}, check_login{});

	server.router<GET>("/edit/*", &Index::edit, nullptr, session_init{}, base_path_aop{}, check_login{});

	server.router<POST>("/saveedit", &Index::saveedit, nullptr, session_init{}, base_path_aop{}, check_login_ajax{});

	server.router<POST>("/delArticle", &Index::delArticle, nullptr, session_init{}, base_path_aop{}, check_login_ajax{});

	server.router<POST>("/addComment", &Index::addComment, nullptr, session_init{}, base_path_aop{}, check_login{});

	server.router<POST, GET>("/regpage", &Index::regpage, nullptr, session_init{}, base_path_aop{});

	server.router<POST, GET>("/reg", &Index::regUser, nullptr, session_init{}, base_path_aop{});

	server.run();
}