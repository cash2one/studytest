-- phpMyAdmin SQL Dump
-- version 4.3.11.1
-- http://www.phpmyadmin.net
--
-- Host: 127.0.0.1
-- Generation Time: 2016-02-18 15:56:02
-- 服务器版本： 5.6.28-log
-- PHP Version: 5.6.17

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `baidu_zhidao`
--
CREATE DATABASE IF NOT EXISTS `baidu_zhidao` DEFAULT CHARACTER SET utf8 COLLATE utf8_bin;
USE `baidu_zhidao`;

-- --------------------------------------------------------

--
-- 表的结构 `answer`
--

DROP TABLE IF EXISTS `answer`;
CREATE TABLE IF NOT EXISTS `answer` (
  `id` bigint(20) unsigned NOT NULL COMMENT '无意义的自增ID',
  `aid` bigint(50) unsigned NOT NULL COMMENT '问题ID',
  `qid` bigint(20) unsigned NOT NULL COMMENT '问题ID',
  `content` text COLLATE utf8_bin NOT NULL COMMENT '回答的内容',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '3' COMMENT '回答类型：1最佳回答、2推荐回答、3其他回答'
) ENGINE=InnoDB AUTO_INCREMENT=106938 DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='回答信息表';

-- --------------------------------------------------------

--
-- 表的结构 `category`
--

DROP TABLE IF EXISTS `category`;
CREATE TABLE IF NOT EXISTS `category` (
  `id` int(10) unsigned NOT NULL COMMENT '无意义的自增ID',
  `cid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '类别Id',
  `cname` varchar(200) COLLATE utf8_bin NOT NULL DEFAULT '' COMMENT '类别名称',
  `parent_cid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '父类ID'
) ENGINE=InnoDB AUTO_INCREMENT=526 DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='分类表';

-- --------------------------------------------------------

--
-- 表的结构 `question`
--

DROP TABLE IF EXISTS `question`;
CREATE TABLE IF NOT EXISTS `question` (
  `id` bigint(20) unsigned NOT NULL COMMENT '无意义的自增ID',
  `qid` bigint(20) unsigned NOT NULL COMMENT '问题ID',
  `title` varchar(500) COLLATE utf8_bin NOT NULL COMMENT '问题标题',
  `content` text COLLATE utf8_bin NOT NULL COMMENT '问题的内容',
  `url` varchar(500) COLLATE utf8_bin NOT NULL COMMENT '问题的详细URL',
  `tag` varchar(500) COLLATE utf8_bin NOT NULL COMMENT '该问题的tag列表'
) ENGINE=InnoDB AUTO_INCREMENT=106938 DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='问题表';

-- --------------------------------------------------------

--
-- 表的结构 `seedword`
--

DROP TABLE IF EXISTS `seedword`;
CREATE TABLE IF NOT EXISTS `seedword` (
  `id` bigint(20) unsigned NOT NULL COMMENT '无意义的自增ID',
  `word` varchar(200) COLLATE utf8_bin NOT NULL DEFAULT '' COMMENT '种子词语'
) ENGINE=InnoDB AUTO_INCREMENT=71743 DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='用于抓取的种子词语';

-- --------------------------------------------------------

--
-- 表的结构 `tag`
--

DROP TABLE IF EXISTS `tag`;
CREATE TABLE IF NOT EXISTS `tag` (
  `id` int(11) unsigned NOT NULL COMMENT '无意义的自增ID',
  `cid` int(10) unsigned NOT NULL COMMENT '标签ID',
  `tag` varchar(200) COLLATE utf8_bin NOT NULL COMMENT '标签名称'
) ENGINE=InnoDB AUTO_INCREMENT=5301 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `answer`
--
ALTER TABLE `answer`
  ADD PRIMARY KEY (`id`), ADD UNIQUE KEY `aid` (`aid`);

--
-- Indexes for table `category`
--
ALTER TABLE `category`
  ADD PRIMARY KEY (`id`), ADD UNIQUE KEY `cid` (`cid`);

--
-- Indexes for table `question`
--
ALTER TABLE `question`
  ADD PRIMARY KEY (`id`), ADD UNIQUE KEY `qid` (`qid`);

--
-- Indexes for table `seedword`
--
ALTER TABLE `seedword`
  ADD PRIMARY KEY (`id`), ADD UNIQUE KEY `word` (`word`);

--
-- Indexes for table `tag`
--
ALTER TABLE `tag`
  ADD PRIMARY KEY (`id`), ADD UNIQUE KEY `cid` (`cid`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `answer`
--
ALTER TABLE `answer`
  MODIFY `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '无意义的自增ID',AUTO_INCREMENT=106938;
--
-- AUTO_INCREMENT for table `category`
--
ALTER TABLE `category`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '无意义的自增ID',AUTO_INCREMENT=526;
--
-- AUTO_INCREMENT for table `question`
--
ALTER TABLE `question`
  MODIFY `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '无意义的自增ID',AUTO_INCREMENT=106938;
--
-- AUTO_INCREMENT for table `seedword`
--
ALTER TABLE `seedword`
  MODIFY `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '无意义的自增ID',AUTO_INCREMENT=71743;
--
-- AUTO_INCREMENT for table `tag`
--
ALTER TABLE `tag`
  MODIFY `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT '无意义的自增ID',AUTO_INCREMENT=5301;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
