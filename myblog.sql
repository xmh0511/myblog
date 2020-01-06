/*
Navicat MySQL Data Transfer

Source Server         : bandwagong
Source Server Version : 50728
Source Host           : 192.243.114.122:3306
Source Database       : myblog

Target Server Type    : MYSQL
Target Server Version : 50728
File Encoding         : 65001

Date: 2020-01-06 21:27:00
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for article_tb
-- ----------------------------
DROP TABLE IF EXISTS `article_tb`;
CREATE TABLE `article_tb` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `user_id` bigint(20) DEFAULT NULL,
  `title` text,
  `tag_id` varchar(20) DEFAULT NULL,
  `create_at` datetime DEFAULT NULL,
  `update_at` datetime DEFAULT NULL,
  `content` text,
  `browse_level` int(11) DEFAULT NULL,
  `article_describe` text,
  `md_content` text,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for browse_level_tb
-- ----------------------------
DROP TABLE IF EXISTS `browse_level_tb`;
CREATE TABLE `browse_level_tb` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) DEFAULT NULL,
  `create_at` datetime DEFAULT NULL,
  `update_at` datetime DEFAULT NULL,
  `level` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=6 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for browse_tb
-- ----------------------------
DROP TABLE IF EXISTS `browse_tb`;
CREATE TABLE `browse_tb` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `ip` varchar(255) DEFAULT NULL,
  `create_at` date DEFAULT NULL,
  `article_id` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=9 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for comment_tb
-- ----------------------------
DROP TABLE IF EXISTS `comment_tb`;
CREATE TABLE `comment_tb` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `user_id` bigint(20) DEFAULT NULL,
  `content` text,
  `create_at` datetime DEFAULT NULL,
  `update_at` datetime DEFAULT NULL,
  `article_id` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for tag_tb
-- ----------------------------
DROP TABLE IF EXISTS `tag_tb`;
CREATE TABLE `tag_tb` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) DEFAULT NULL,
  `create_at` datetime DEFAULT NULL,
  `update_at` datetime DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for user_tb
-- ----------------------------
DROP TABLE IF EXISTS `user_tb`;
CREATE TABLE `user_tb` (
  `id` bigint(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) DEFAULT NULL,
  `password` varchar(255) DEFAULT NULL,
  `create_at` datetime DEFAULT NULL,
  `update_at` datetime DEFAULT NULL,
  `email` varchar(255) DEFAULT NULL,
  `browse_level` int(11) NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=utf8;
