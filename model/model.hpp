#pragma once
#include "../xmart.hpp"
using namespace xmart;

struct user_tb {
	mysql::BigInt id;
	std::string name;
	std::string password;
	mysql::MysqlDateTime create_at;
	mysql::MysqlDateTime update_at;
	std::string email;
	mysql::Integer browse_level;
};
REFLECTION(user_tb, id, name, password, create_at, update_at, email, browse_level)

struct article_tb {
	mysql::BigInt id;
	mysql::BigInt user_id;
	std::string title;
	std::string tag_id;
	mysql::MysqlDateTime create_at;
	mysql::MysqlDateTime update_at;
	std::string content;
	mysql::Integer browse_level;
	std::string article_describe;
	std::string md_content;
};
REFLECTION(article_tb, id, user_id, title, tag_id, create_at, update_at, content, browse_level, article_describe, md_content)

struct tag_tb {
	mysql::BigInt id;
	std::string name;
	mysql::MysqlDateTime create_at;
	mysql::MysqlDateTime update_at;
};
REFLECTION(tag_tb, id, name, create_at, update_at)

struct comment_tb {
	mysql::BigInt id;
	mysql::BigInt user_id;
	std::string content;
	mysql::MysqlDateTime create_at;
	mysql::MysqlDateTime update_at;
	mysql::BigInt article_id;
};
REFLECTION(comment_tb,id, user_id, content, create_at, update_at, article_id)

struct browse_level_tb {
	mysql::Integer id;
	std::string name;
	mysql::MysqlDateTime create_at;
	mysql::MysqlDateTime update_at;
	mysql::Integer level;
};
REFLECTION(browse_level_tb,id,name, create_at, update_at, level)

struct browse_tb {
	mysql::Integer id;
	std::string ip;
	mysql::MysqlDate create_at;
	mysql::BigInt article_id;
};
REFLECTION(browse_tb,id,ip, create_at, article_id)


