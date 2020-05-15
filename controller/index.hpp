#pragma once
#include "../xmart.hpp"
#include "../model/model.hpp"
using namespace xmart;
class Index {
public:
	template<typename T>
	struct page_data {
		int page_size;
		int next_page;
		int page_count;
		int total_size;
		int now_page;
		std::vector<T> data;
	};
private:
	std::vector<tag_tb> get_tags() {
		dao_t<mysql> dao;
		if (dao.is_open()) {
			auto tags = dao.query<tag_tb>("");
			if (tags.first) {
				return tags.second;
			}
		}
		return {};
	}
	std::vector<browse_level_tb> get_browse_levels() {
		dao_t<mysql> dao;
		if (dao.is_open()) {
			auto level = dao.query<browse_level_tb>("");
			if (level.first) {
				return level.second;
			}
		}
		return {};
	}
	std::vector<std::string> get_tag_names(std::vector<nonstd::string_view> ids) {
		dao_t<mysql> dao;
		std::vector<std::string> result;
		if (dao.is_open()) {
			for (auto& iter : ids) {
				auto tags = dao.query<tag_tb>("where id=?", view2str(iter));
				if (!tags.second.empty()) {
					result.push_back(tags.second[0].name);
				}
			}
		}
		return result;
	}
	std::string join(std::vector<std::string> const& vec, std::string const& text) {
		std::string result;
		for (int i = 0; i < vec.size(); i++) {
			if (i < vec.size() - 1) {
				result.append(vec[i]);
				result.append(text);
				continue;
			}
			result.append(vec[i]);
		}
		return result;
	}
private:
	page_data<article_tb> get_article_list(std::string const& user_id, int pageSize, int pageNumber, std::string const& tags = "") {
		if (pageSize <= 0) {
			pageSize = 4;
		}
		if (pageNumber < 1) {
			pageNumber = 1;
		}
		std::string condition = "";
		std::string totalcondition = "";
		std::string tagscondition = "";
		auto tags_view = split(nonstd::string_view(tags.data(), tags.size()), ",");
		for (auto index = 0; index < tags_view.size(); index++) { //如果有tag 拼凑条件
			auto c = tags_view[index];
			if (index < tags_view.size() - 1) {
				tagscondition += "'%" + view2str(c) + "%' or ";
				continue;
			}
			tagscondition += "'%" + view2str(c) + "%'";
		}
		if (user_id != "0") {
			condition = "where user_id='" + user_id + "'";
			if (!tagscondition.empty()) {
				condition += " and  tag_id like " + tagscondition;
			}
		}
		else {  //all
			condition = "where browse_level<>100 ";
			if (!tagscondition.empty()) {
				condition += " and  tag_id like " + tagscondition;
			}
		}
		totalcondition = condition;
		condition += " order by update_at desc limit " + std::to_string((pageNumber - 1)*pageSize) + " , " + std::to_string(pageSize);
		dao_t<mysql> dao;
		if (dao.is_open()) {
			auto result = dao.query<article_tb>(condition);
			auto totalr = dao.query<article_tb>(totalcondition);
			int total_size = totalr.second.size();
			int per_page = pageSize;
			auto diff = total_size / pageSize;
			int page_count = diff + (total_size%pageSize == 0 ? 0 : 1);
			auto next_page = (pageNumber + 1) > page_count ? pageNumber : pageNumber + 1;
			page_data<article_tb> r{ pageSize ,next_page,page_count ,total_size ,pageNumber,result.second };
			return r;
		}
		return page_data<article_tb>{4, 1, 0, 0, 1, {}};
	}

	bool check_level(std::string const& user_id, std::string const& article_id) {
		dao_t<mysql> dao;
		if (dao.is_open()) {
			auto result = dao.query<article_tb>("where id=?", article_id);
			auto user = dao.query<user_tb>("where id=?", user_id);
			if (result.second.empty()) {
				return false;
			}
			auto& article = result.second[0];
			auto& userinfo = user.second[0];
			auto check_level = userinfo.browse_level.value() >= article.browse_level.value();
			if (userinfo.id.value() == article.user_id.value() || check_level) {
				return true;
			}
			return false;
		}
		return false;
	}

	json get_comment_list_json(std::string const& article_id) {
		dao_t<mysql> dao;
		json list = json::array();
		if (dao.is_open()) {
			auto r = dao.query<comment_tb>("where article_id=?", article_id);
			for (auto& iter : r.second) {
				json node;
				auto users = dao.query<user_tb>("where id=?", std::to_string(iter.user_id.value()));
				node["name"] = users.second[0].name;
				node["time"] = iter.create_at.value();
				node["content"] = iter.content;
				list.push_back(node);
			}
		}
		return list;
	}
	void add_browse_count(std::string const& ip, std::string const& article_id) {
		if (ip.empty()) {
			return;
		}
		dao_t<mysql> dao;
		if (dao.is_open()) {
			auto r = dao.query<browse_tb>("where article_id=? and ip=?", article_id, ip);
			if (r.second.empty()) {
				browse_tb info;
				info.id = 0;
				info.ip = ip;
				info.article_id = std::atoi(article_id.data());
				info.create_at.format_timestamp(std::time(nullptr));
				dao.insert(info);
			}
		}
	}
	std::int64_t get_browse_count(std::string const& article_id) {
		dao_t<mysql> dao;
		if (dao.is_open()) {
			auto r = dao.query<browse_tb>("where article_id=?", article_id);
			return r.second.size();
		}
		return 0;
	}
	std::string get_browse_level_name(std::string const& browse_level) {
		dao_t<mysql> dao;
		if (dao.is_open()) {
			auto r = dao.query<browse_level_tb>("where level=?", browse_level);
			if (!r.second.empty()) {
				return r.second[0].name;
			}
		}
		return "";
	}
public:
	void index(request& req, response& res) {
		auto pageSize = req.param("pageSize");
		auto pageNumber = req.param("pageNumber");
		auto tagsView = req.param("tag");
		auto result = get_article_list("0", std::atoi(pageSize.data()), std::atoi(pageNumber.data()), view2str(tagsView));
		res.set_attr("tag_list", list_to_json(get_tags()));
		json list = json::array();
		for (auto& iter : result.data) {
			json node;
			node["describe"] = iter.article_describe;
			node["update_at"] = iter.update_at.value();
			node["title"] = iter.title;
			dao_t<mysql> dao;
			auto user = dao.query<user_tb>("where id=?", std::to_string(iter.user_id.value()));
			auto name = user.second[0].name;
			node["author"] = name;
			node["id"] = iter.id.value();
			auto artid = std::to_string(iter.id.value());
			auto tags = get_tag_names(split(nonstd::string_view(iter.tag_id.data(), iter.tag_id.size()), ","));
			node["tag"] = tags.empty() == false ? tags[0] : "";
			node["comment_size"] = get_comment_list_json(artid).size();
			node["browse_count"] = get_browse_count(artid);
			node["browse_level_name"] = get_browse_level_name(std::to_string(iter.browse_level.value()));
			list.push_back(node);
		}
		res.set_attr("article_list", list);
		auto calc = result.now_page - 1;
		auto forward_page = calc == 0 ? 1 : calc;
		res.set_attr("forward_page", forward_page);
		res.set_attr("page_size", result.page_size);
		res.set_attr("page_count", result.page_count);
		res.set_attr("next_page", result.next_page);
		res.set_attr("total_size", result.total_size);
		res.set_attr("now_page", result.now_page);
		res.set_attr("tag", view2str(tagsView));
		res.write_view("./www/index.html",true);
	}

	void blog(request& req, response& res) {
		auto& session = req.session("XMART_BLOG");
		auto user_id = session.get_data<std::int64_t>("user_id");
		auto pageSize = req.param("pageSize");
		auto pageNumber = req.param("pageNumber");
		dao_t<mysql> dao;
		json list = json::array();
		if (dao.is_open()) {
			/*auto result = dao.query<article_tb>("where user_id='" + std::to_string(user_id) + "'");*/
			auto result = get_article_list(std::to_string(user_id), std::atoi(pageSize.data()), std::atoi(pageNumber.data()));
			auto user = dao.query<user_tb>("where id=?", std::to_string(user_id));
			auto name = user.second[0].name;
			for (auto& iter : result.data) {
				json node;
				node["describe"] = iter.article_describe;
				node["update_at"] = iter.update_at.value();
				node["title"] = iter.title;
				node["author"] = name;
				node["id"] = iter.id.value();
				auto artid = std::to_string(iter.id.value());
				auto tags = get_tag_names(split(nonstd::string_view(iter.tag_id.data(), iter.tag_id.size()), ","));
				node["tag"] = tags.empty() == false ? tags[0] : "";
				node["comment_size"] = get_comment_list_json(artid).size();
				node["browse_count"] = get_browse_count(artid);
				node["browse_level_name"] = get_browse_level_name(std::to_string(iter.browse_level.value()));
				list.push_back(node);
			}
			res.set_attr("article_list", list);
			auto calc = result.now_page - 1;
			auto forward_page = calc == 0 ? 1 : calc;
			res.set_attr("forward_page", forward_page);
			res.set_attr("page_size", result.page_size);
			res.set_attr("page_count", result.page_count);
			res.set_attr("next_page", result.next_page);
			res.set_attr("total_size", result.total_size);
			res.set_attr("now_page", result.now_page);
		}
		else {
			res.set_attr("article_list", list);
			res.set_attr("forward_page", 1);
			res.set_attr("page_size", 4);
			res.set_attr("page_count", 0);
			res.set_attr("next_page", 1);
			res.set_attr("total_size", 0);
			res.set_attr("now_page", 1);
		}
		res.write_view("./www/blog.html",true);
	}

	void addblog(request& req, response& res) {
		res.set_attr("tag_list", list_to_json(get_tags()));
		res.set_attr("browse_level_list", list_to_json(get_browse_levels()));
		auto result0 = get_article_list("0", 4, 1);
		res.set_attr("article_list", list_to_json(result0.data));
		res.write_view("./www/addblog.html",true);
	}

	void addarticle(request& req, response& res) {
		auto body = req.body();
		nlohmann::json message;
		try {
			auto& session = req.session("XMART_BLOG");
			auto user_id = session.get_data<std::int64_t>("user_id");
			auto json = json::parse(view2str(body));
			article_tb info;
			info.id = 0;
			info.content = json["content"].get<std::string>();
			info.md_content = json["md_content"].get<std::string>();
			info.tag_id = json["tags"].get<std::string>();
			info.title = json["title"].get<std::string>();
			info.article_describe = json["describe"].get<std::string>();
			auto level = json["level"].get<std::string>();
			int level_ = std::atoi(level.data());
			info.browse_level = level_ > 0 ? level_ : 1;
			info.create_at.format_timestamp(std::time(nullptr));
			info.update_at = info.create_at;
			info.user_id = user_id;
			dao_t<mysql> dao;
			if (info.content.empty() || info.md_content.empty() || info.tag_id.empty() || info.title.empty() || info.article_describe.empty()) {
				message["success"] = false;
				message["message"] = "新增失败,请填写完整信息";
				res.write_json(message,true);
				return;
			}
			if (dao.is_open()) {
				auto result = dao.insert(info);
				if (result.first) {
					message["success"] = true;
				}
				else {
					message["success"] = false;
					message["message"] = "新增失败";
				}
			}
		}
		catch (std::exception const& ec) {
			message["success"] = false;
			message["message"] = ec.what();
		}
		res.write_json(message, true);
	}

	void detail(request& req, response& res) {
		auto id = req.param(0);
		auto number = std::atoi(id.data());
		if (id.empty() || number <= 0) {
			res.set_attr("state", false);
			std::string msg = "文章已删除或不存在";
			res.set_attr("msg", msg);
			res.write_view("./www/single.html", true);
			return;
		}
		dao_t<mysql> dao;
		if (dao.is_open()) {
			auto result = dao.query<article_tb>("where id=?", std::to_string(number));
			if (result.second.empty()) {
				res.set_attr("state", false);
				std::string msg = "文章已删除或不存在";
				res.set_attr("msg", msg);
				res.write_view("./www/single.html", true);
				return;
			}
			else {
				auto& article = result.second[0];
				auto author = dao.query<user_tb>("where id=?", std::to_string(article.user_id.value()));
				if (article.browse_level.value() == 1) {
					auto artid = std::to_string(number);
					auto is_proxy = res.get_attr<bool>("proxy");
					std::string browse_ip = is_proxy ? view2str(req.header("X-Real-IP")) : req.connection().remote_ip();
					add_browse_count(browse_ip, artid);
					auto result0 = get_article_list("0", 4, 1);
					res.set_attr("article_list", list_to_json(result0.data));
					res.set_attr("tag_list", list_to_json(get_tags()));
					res.set_attr("state", true);
					res.set_attr("article_data", map_to_json(article));
					res.set_attr("author", author.second[0].name);
					auto tags = get_tag_names(split(nonstd::string_view(article.tag_id.data(), article.tag_id.size()), ","));
					res.set_attr("comment_list", get_comment_list_json(artid));
					res.set_attr("tags", join(tags, ","));
					res.set_attr("browse_count", get_browse_count(artid));
					res.write_view("./www/single.html", true);
					return;
				}
				auto& session = req.session("XMART_BLOG");
				if (session.empty() || session.get_data<std::string>("islogin") != "true") {
					res.write_view("./www/login.html", true);
					return;
				}
				auto user_id = session.get_data<std::int64_t>("user_id");
				auto user = dao.query<user_tb>("where id=?", std::to_string(user_id));
				auto& userinfo = user.second[0];
				auto check_level = userinfo.browse_level.value() >= article.browse_level.value();
				auto result0 = get_article_list("0", 4, 1);
				res.set_attr("article_list", list_to_json(result0.data));
				res.set_attr("tag_list", list_to_json(get_tags()));
				if (userinfo.id.value() == article.user_id.value() || check_level) {
					auto artid = std::to_string(number);
					auto is_proxy = res.get_attr<bool>("proxy");
					std::string browse_ip = is_proxy ? view2str(req.header("X-Real-IP")) : req.connection().remote_ip();
					add_browse_count(browse_ip, artid);
					res.set_attr("state", true);
					res.set_attr("article_data", map_to_json(article));
					res.set_attr("author", author.second[0].name);
					auto tags = get_tag_names(split(nonstd::string_view(article.tag_id.data(), article.tag_id.size()), ","));
					res.set_attr("comment_list", get_comment_list_json(artid));
					res.set_attr("tags", join(tags, ","));
					res.set_attr("browse_count", get_browse_count(artid));
					res.write_view("./www/single.html",true);
					return;
				}
				else {
					res.set_attr("state", false);
					std::string msg = "阅读权限不够";
					res.set_attr("msg", msg);
					res.write_view("./www/single.html", true);
					return;
				}
			}
		}

	}

	void edit(request& req, response& res) {
		auto id = req.param(0);
		auto& session = req.session("XMART_BLOG");
		auto user_id = session.get_data<std::int64_t>("user_id");
		dao_t<mysql> dao;
		if (dao.is_open()) {
			auto number = std::atoi(id.data());
			auto check_article_id = std::to_string(number);
			auto result = dao.query<article_tb>("where id=? and user_id=?" ,check_article_id ,std::to_string(user_id));
			if (result.second.empty()) {
				res.set_attr("state", false);
				std::string msg = "非法编辑";
				res.set_attr("msg", msg);
				res.write_view("./www/edit.html", true);
				return;
			}
			auto& article = result.second[0];
			res.set_attr("state", true);
			res.set_attr("article", map_to_json(article));
			res.set_attr("tag_list", list_to_json(get_tags()));
			res.set_attr("browse_level_list", list_to_json(get_browse_levels()));
			auto result0 = get_article_list("0", 4, 1);
			res.set_attr("article_list", list_to_json(result0.data));
			res.write_view("./www/edit.html", true);
			return;
		}
		std::string msg = "服务器繁忙";
		res.set_attr("state", false);
		res.write_view("./www/edit.html", true);
	}

	void saveedit(request& req, response& res) {
		auto body = req.body();
		auto& session = req.session("XMART_BLOG");
		auto user_id = session.get_data<std::int64_t>("user_id");
		json message;
		try {
			auto data = json::parse(view2str(body));
			dao_t<mysql> dao;
			if (dao.is_open()) {
				auto id = data["id"].get<std::string>();
				auto number = std::atoi(id.data());
				auto check_article_id = std::to_string(number);
				auto result = dao.query<article_tb>("where id=? and user_id=? " ,check_article_id , std::to_string(user_id));
				if (result.second.empty()) {
					message["success"] = false;
					message["message"] = "无效的编辑";
					res.write_json(message, true);
					return;
				}
				auto article = result.second[0];
				article.article_describe = data["describe"].get<std::string>();
				article.title = data["title"].get<std::string>();
				auto level = data["level"].get<std::string>();
				int level_ = std::atoi(level.data());
				article.browse_level = level_ > 0 ? level_ : 1;
				article.md_content = data["md_content"].get<std::string>();
				article.content = data["content"].get<std::string>();
				article.tag_id = data["tags"].get<std::string>();
				article.update_at.format_timestamp(std::time(nullptr));
				bool r = dao.update(article);
				message["success"] = r;
				if (!r) {
					message["success"] = "更新失败";
				}
				res.write_json(message, true);
				return;
			}
			message["success"] = false;
			message["message"] = "服务器繁忙";
			res.write_json(message, true);
		}
		catch (...) {
			message["success"] = false;
			message["message"] = "编辑数据不合法";
			res.write_json(message,true);
			return;
		}
	}

	void delArticle(request& req, response& res) {
		auto& session = req.session("XMART_BLOG");
		auto user_id = session.get_data<std::int64_t>("user_id");
		auto id = req.query("id");
		json message;
		dao_t<mysql> dao;
		if (dao.is_open()) {
			auto number = std::atoi(id.data());
			auto check_article_id = std::to_string(number);
			auto result = dao.query<article_tb>("where id=? and user_id=? ", check_article_id ,std::to_string(user_id));
			if (result.second.empty()) {
				message["success"] = false;
				message["message"] = "无效的删除";
				res.write_json(message, true);
				return;
			}
			auto r = dao.del<article_tb>("where id=?", check_article_id);
			message["success"] = r.first;
			if (!r.first) {
				message["message"] = "删除失败";
			}
			res.write_json(message,true);
			return;
		}
		message["success"] = false;
		message["message"] = "服务器繁忙";
		res.write_json(message, true);
	}

	void addComment(request& req, response& res) {
		auto id = req.query("id");
		auto comment = req.query("comment");
		auto& session = req.session("XMART_BLOG");
		auto user_id = session.get_data<std::int64_t>("user_id");
		if (comment.empty()) {
			res.redirect(res.get_attr<std::string>("base_path") + "/detail/" + view2str(id));
			return;
		}
		if (check_level(std::to_string(user_id), view2str(id))) {
			comment_tb info;
			info.id = 0;
			info.article_id = std::atoi(id.data());
			info.content = view2str(comment);
			info.user_id = user_id;
			info.create_at.format_timestamp(std::time(nullptr));
			info.update_at = info.create_at;
			dao_t<mysql> dao;
			auto r = dao.insert(info);
		}
		res.redirect(res.get_attr<std::string>("base_path") + "/detail/" + view2str(id));
	}

	void regUser(request& req, response& res) {
		user_tb info;
		auto name = view2str(req.query("name"));
		auto password = view2str(req.query("password"));
		auto repassword = view2str(req.query("repassword"));
		if (name.empty() || password.empty()) {
			res.set_attr("state", true);
			res.write_view("./www/reg.html");
			return;
		}
		if (password != repassword) {
			res.set_attr("state", false);
			res.set_attr("msg", "两次输入的密码不一致");
			res.write_view("./www/reg.html", true);
			return;
		}
		dao_t<mysql> dao;
		if (dao.is_open()) {
			auto r = dao.query<user_tb>("where name=?" , name);
			if (r.second.empty()) {
				info.browse_level = 1;
				info.create_at.format_timestamp(std::time(nullptr));
				info.update_at = info.create_at;
				info.email = view2str(req.query("email"));
				info.name = name;
				info.id = 0;
				info.password = MD5(password).toStr();
				auto success = dao.insert(info);
				res.write_view("./www/login.html", true);
				return;
			}
			else {
				res.set_attr("state", false);
				res.set_attr("msg", "用户名已存在");
				res.write_view("./www/reg.html", true);
				return;
			}
		}
		res.write_view("./www/reg.html",true);
	}

	void regpage(request& req, response& res) {
		res.write_view("./www/reg.html", true);
	}

	void loginPage(request& req, response& res) {
		res.write_view("./www/login.html", true);
	}

	void upload(request& req, response& res) {
		auto& file = req.file("editormd-image-file");
		json root;
		if (file.is_exsit()) {
			root["success"] = 1;
			root["message"] = "";
			auto base_path = res.get_attr<std::string>("base_path");
			root["url"] = base_path + file.url_path();
		}
		else {
			root["success"] = 0;
			root["message"] = "上传失败"; 
		}
		res.write_json(root, true);
	}

	void login(request& req, response& res) {
		auto& session = req.session("XMART_BLOG");
		auto name = req.query("name");
		auto password = req.query("password");
		auto md5pass = MD5(view2str(password)).toStr();
		if (name.empty() || password.empty()) {
			res.write_view("./www/login.html", true);
			return;
		}
		dao_t<mysql> dao;
		//std::cout << "dao.is_open():" << dao.is_open() << std::endl;
		if (dao.is_open()) {
			auto result = dao.query<user_tb>("where name=?", view2str(name));
			//std::cout << "result.second.empty():" << result.second.empty() << std::endl;
			if (!result.second.empty()) {
				auto& data = result.second[0];
				//std::cout << "sql pass: " << data.password << " input pass: " << md5pass << std::endl;
				if (data.password == md5pass) {
					//std::cout << "password equit" << std::endl;
					session.set_data("user_id", data.id.value());
					session.set_data("islogin", "true");
					session.set_expires(28800);
					auto base_path = res.get_attr<std::string>("base_path");
					session.get_cookie().set_path(base_path);
					res.redirect(base_path + "/myblog");
					return;
				}
			}
			res.set_attr("state", false);
			res.set_attr("msg", "用户名或密码不正确");
			res.write_view("./www/login.html", true);
			return;
		}
		res.set_attr("state", false);
		res.set_attr("msg", "服务器繁忙");
		res.write_view("./www/login.html", true);
	}
};