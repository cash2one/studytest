-- phpMyAdmin SQL Dump
-- version 4.3.11.1
-- http://www.phpmyadmin.net
--
-- Host: 127.0.0.1
-- Generation Time: 2016-03-27 19:59:59
-- 服务器版本： 5.6.28
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

-- --------------------------------------------------------

--
-- 表的结构 `answer`
--

CREATE TABLE IF NOT EXISTS `answer` (
  `id` bigint(20) unsigned NOT NULL COMMENT '无意义的自增ID',
  `aid` bigint(50) unsigned NOT NULL COMMENT '问题ID',
  `qid` bigint(20) unsigned NOT NULL COMMENT '问题ID',
  `content` text COLLATE utf8_bin NOT NULL COMMENT '回答的内容',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '3' COMMENT '回答类型：1最佳回答、2推荐回答、3其他回答'
) ENGINE=InnoDB AUTO_INCREMENT=100447785 DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='回答信息表';

-- --------------------------------------------------------

--
-- 表的结构 `category`
--

CREATE TABLE IF NOT EXISTS `category` (
  `id` int(10) unsigned NOT NULL COMMENT '无意义的自增ID',
  `cid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '类别Id',
  `cname` varchar(200) COLLATE utf8_bin NOT NULL DEFAULT '' COMMENT '类别名称',
  `parent_cid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '父类ID'
) ENGINE=InnoDB AUTO_INCREMENT=176 DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='分类表';

-- --------------------------------------------------------

--
-- 表的结构 `question`
--

CREATE TABLE IF NOT EXISTS `question` (
  `id` bigint(20) unsigned NOT NULL COMMENT '无意义的自增ID',
  `qid` bigint(20) unsigned NOT NULL COMMENT '问题ID',
  `title` varchar(500) COLLATE utf8_bin NOT NULL COMMENT '问题标题',
  `content` text COLLATE utf8_bin NOT NULL COMMENT '问题的内容',
  `url` varchar(500) COLLATE utf8_bin NOT NULL COMMENT '问题的详细URL',
  `tag` varchar(500) COLLATE utf8_bin NOT NULL COMMENT '该问题的tag列表'
) ENGINE=InnoDB AUTO_INCREMENT=100446980 DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='问题表';

-- --------------------------------------------------------

--
-- 表的结构 `seedword`
--

CREATE TABLE IF NOT EXISTS `seedword` (
  `id` bigint(20) unsigned NOT NULL COMMENT '无意义的自增ID',
  `word` varchar(200) COLLATE utf8_bin NOT NULL DEFAULT '' COMMENT '种子词语'
) ENGINE=InnoDB AUTO_INCREMENT=23750020 DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='用于抓取的种子词语';

-- --------------------------------------------------------

--
-- 表的结构 `tag`
--

CREATE TABLE IF NOT EXISTS `tag` (
  `id` int(11) unsigned NOT NULL COMMENT '无意义的自增ID',
  `cid` int(10) unsigned NOT NULL COMMENT '标签ID',
  `tag` varchar(200) COLLATE utf8_bin NOT NULL COMMENT '标签名称'
) ENGINE=InnoDB AUTO_INCREMENT=2617 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- 表的结构 `topword`
--

CREATE TABLE IF NOT EXISTS `topword` (
  `id` int(10) unsigned NOT NULL COMMENT '无意义的自增ID',
  `word` varchar(250) COLLATE utf8_bin NOT NULL DEFAULT ''
) ENGINE=InnoDB AUTO_INCREMENT=28794 DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='从各搜索网站上抓取过来的热搜词';

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
-- Indexes for table `topword`
--
ALTER TABLE `topword`
  ADD PRIMARY KEY (`id`), ADD UNIQUE KEY `word` (`word`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `answer`
--
ALTER TABLE `answer`
  MODIFY `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '无意义的自增ID',AUTO_INCREMENT=100447785;
--
-- AUTO_INCREMENT for table `category`
--
ALTER TABLE `category`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '无意义的自增ID',AUTO_INCREMENT=176;
--
-- AUTO_INCREMENT for table `question`
--
ALTER TABLE `question`
  MODIFY `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '无意义的自增ID',AUTO_INCREMENT=100446980;
--
-- AUTO_INCREMENT for table `seedword`
--
ALTER TABLE `seedword`
  MODIFY `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '无意义的自增ID',AUTO_INCREMENT=23750020;
--
-- AUTO_INCREMENT for table `tag`
--
ALTER TABLE `tag`
  MODIFY `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT '无意义的自增ID',AUTO_INCREMENT=2617;
--
-- AUTO_INCREMENT for table `topword`
--
ALTER TABLE `topword`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '无意义的自增ID',AUTO_INCREMENT=28794;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
