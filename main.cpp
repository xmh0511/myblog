#include "xmart.hpp"
using namespace xmart;

#include "./model/model.hpp"
#include "./controller/index.hpp"
struct wwwConfig {
public:
	static wwwConfig& get() {
		static wwwConfig instance;
		return instance;
	}
public:
	bool open(std::string const& fileName) {
		std::ifstream file(fileName);
		if (!file.is_open()) {
			return false;
		}
		std::stringstream ss;
		ss << file.rdbuf();
		config_ = json::parse(ss.str());
		return true;
	}
	json& config() {
		return config_;
	}
private:
	json config_;
};
struct session_init {
	bool before(request& req, response& res) {
		auto& session = req.session("XMART_BLOG");
		if (session.empty()) {
			auto& json = wwwConfig::get().config();
			auto base_path = json["base_path"].get<std::string>();
			auto& session0 = req.create_session("XMART_BLOG");
			session0.get_cookie().set_path(base_path);
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
		auto& json = wwwConfig::get().config();
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
			res.write_file_view("./www/login.html",true);
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
			if (req.get_content_type() == content_type::multipart_form) {
				req.remove_all_upload_files();
			}
			json message;
			message["success"] = false;
			message["code"] = 404;
			message["message"] = "用户认证失败";
			res.write_json(message,true);
			return false;
		}
		return true;
	}
	bool after(request& req, response& res) {
		return true;
	}
};

struct  disableUpload {
	bool prehandle(request& req, response& res) {
		if (req.get_content_type() == content_type::multipart_form) {
			res.write_file_view("./www/error.html", true, http_status::bad_request);
			return false;
		}
		return true;
	}
	bool posthandle() {
		return true;
	}
};

struct testInteceptor {
	bool prehandle(request& req, response& res) {
		using namespace nonstd::literals;
		auto id = req.param("id");
		if (id=="0"_sv) {
			res.write_file_view("./www/error.html", true, http_status::bad_request);
			return false;
		}
		return true;
	}
	bool posthandle() {
		return true;
	}
};

int main() {
	bool r = false;
    auto smart_ptr = init_xmart("./config.json", r, [](std::string const& message) {
		std::cout << message << std::endl;
	});
	if (!r) {
		std::cout<<"config has some error"<<std::endl;
		return 0;
	}

	if (!wwwConfig::get().open("./www.config")) {
		std::cout << "www.config does not exist" << std::endl;
		return  0;
	} 

	http_server& server = *smart_ptr;


	server.add_view_method("calcNumber", 2, [](inja::Arguments& args)->json {
		auto src = args.at(0)->get<int>();
		auto number = args.at(1)->get<int>();
		auto r = src + number;
		return r;
	});

	server.router<GET>("/", &Index::index, session_init{}, base_path_aop{}, disableUpload{});

	server.router<POST,GET>("/login", &Index::login,  session_init{}, base_path_aop{}, disableUpload{});

	server.router<GET>("/myblog", &Index::blog,  session_init{}, base_path_aop{}, check_login{}, disableUpload{});

	server.router<GET>("/addblog", &Index::addblog,  session_init{}, base_path_aop{}, check_login{}, disableUpload{});

	server.router<POST>("/addarticle", &Index::addarticle,  session_init{}, base_path_aop{}, check_login_ajax{}, disableUpload{});

	server.router<GET>("/detail/*", &Index::detail,  session_init{}, base_path_aop{}, disableUpload{});

	server.router<GET>("/edit/*", &Index::edit,  session_init{}, base_path_aop{}, check_login{}, disableUpload{});

	server.router<POST>("/saveedit", &Index::saveedit,  session_init{}, base_path_aop{}, check_login_ajax{}, disableUpload{});

	server.router<POST>("/delArticle", &Index::delArticle,  session_init{}, base_path_aop{}, check_login_ajax{}, disableUpload{});

	server.router<POST>("/addComment", &Index::addComment,  session_init{}, base_path_aop{}, check_login{}, disableUpload{});

	server.router<POST, GET>("/regpage", &Index::regpage,  session_init{}, base_path_aop{}, disableUpload{});

	server.router<POST, GET>("/reg", &Index::regUser,  session_init{}, base_path_aop{}, disableUpload{});

	server.router<POST>("/upload", &Index::upload,  base_path_aop{}, check_login_ajax{});

	server.router<GET,POST>("/performance", [](request& req, response& res) {
		res.write_string("ok", true);
		}, testInteceptor{});

	server.run();
}