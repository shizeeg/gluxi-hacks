-- MySQL dump 10.10
--
-- Host: testing    Database: gluxi
-- ------------------------------------------------------
-- Server version	5.0.22-Debian_3-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `aliases`
--

DROP TABLE IF EXISTS `aliases`;
CREATE TABLE `aliases` (
  `plugin` tinyint(4) NOT NULL,
  `storage` tinyint(4) NOT NULL,
  `name` varchar(50) collate utf8_bin NOT NULL,
  `value` varchar(200) collate utf8_bin NOT NULL,
  PRIMARY KEY  (`plugin`,`storage`,`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

--
-- Table structure for table `conference_alists`
--

DROP TABLE IF EXISTS `conference_alists`;
CREATE TABLE `conference_alists` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `conference_id` int(11) NOT NULL,
  `list` tinyint(1) NOT NULL,
  `matcher` tinyint(4) NOT NULL DEFAULT '0',
  `test` tinyint(4) NOT NULL DEFAULT '0',
  `inv` tinyint(1) NOT NULL DEFAULT '0',
  `value` varchar(50) collate utf8_bin NOT NULL,
  `reason` varchar(100) collate utf8_bin NULL,
  `expire` datetime default NULL,
  PRIMARY KEY (id),
  UNIQUE KEY  (`conference_id`,`list`,`matcher`, `test`, `value`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

--
-- Table structure for table `conference_jids`
--

DROP TABLE IF EXISTS `conference_jids`;
CREATE TABLE `conference_jids` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `conference_id` int(11) NOT NULL,
  `jid` varchar(50) collate utf8_bin NOT NULL,
  `resource` varchar(50) collate utf8_bin default NULL,
  `temporary` tinyint(1) NOT NULL default '0',
  `created` datetime default NULL,
  PRIMARY KEY  (`id`),
  KEY `jid` (`conference_id`,`jid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

--
-- Table structure for table `conference_nicks`
--

DROP TABLE IF EXISTS `conference_nicks`;
CREATE TABLE `conference_nicks` (
  `id` int(11) NOT NULL auto_increment,
  `conference_id` int(11) NOT NULL,
  `nick` varchar(50) collate utf8_bin NOT NULL,
  `jid` int(11) NOT NULL default '0',
  `created` datetime NOT NULL,
  `online` tinyint(1) NOT NULL default '0',
  `joined` datetime NOT NULL,
  `lastaction` datetime NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

--
-- Table structure for table `conferences`
--

DROP TABLE IF EXISTS `conferences`;
CREATE TABLE `conferences` (
  `name` varchar(50) collate utf8_bin NOT NULL,
  `id` int(11) NOT NULL auto_increment,
  `nick` varchar(50) collate utf8_bin NOT NULL,
  `created` datetime NOT NULL,
  `autojoin` tinyint(1) NOT NULL default '1',
  `online` tinyint(1) NOT NULL default '0',
  `joined` datetime default NULL,
  PRIMARY KEY  (`name`),
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

--
-- Table structure for table `webstatus`
--

DROP TABLE IF EXISTS `webstatus`;
CREATE TABLE `webstatus` (
  `jid` varchar(50) collate utf8_unicode_ci NOT NULL,
  `hash` varchar(50) collate utf8_unicode_ci NOT NULL,
  `status` varchar(15) collate utf8_unicode_ci default NULL,
  `available` varchar(100) collate utf8_unicode_ci default NULL,
  `away` varchar(100) collate utf8_unicode_ci default NULL,
  `chat` varchar(100) collate utf8_unicode_ci default NULL,
  `dnd` varchar(100) collate utf8_unicode_ci default NULL,
  `unavailable` varchar(100) collate utf8_unicode_ci default NULL,
  `xa` varchar(100) collate utf8_unicode_ci default NULL,
  PRIMARY KEY  (`jid`),
  UNIQUE KEY `hash` (`hash`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Table structure for table `words`
--

DROP TABLE IF EXISTS `words`;
CREATE TABLE `words` (
  `plugin` tinyint(4) NOT NULL,
  `storage` tinyint(4) NOT NULL,
  `name` varchar(50) collate utf8_unicode_ci NOT NULL,
  `nick` varchar(50) collate utf8_unicode_ci NOT NULL,
  `date` datetime NOT NULL,
  `value` blob NOT NULL,
  PRIMARY KEY  (`plugin`,`storage`,`name`,`nick`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2006-12-31  2:18:16
-- MySQL dump 10.11
--
-- Host: localhost    Database: test
-- ------------------------------------------------------
-- Server version	5.0.32-Debian_7etch5-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `configuration`
--

DROP TABLE IF EXISTS `configuration`;
CREATE TABLE `configuration` (
  `plugin` tinyint(4) NOT NULL,
  `storage` tinyint(4) NOT NULL,
  `name` varchar(50) collate utf8_unicode_ci NOT NULL,
  `value` varchar(250) collate utf8_unicode_ci default NULL,
  PRIMARY KEY  (`plugin`,`storage`,`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Dumping data for table `configuration`
--

LOCK TABLES `configuration` WRITE;
/*!40000 ALTER TABLE `configuration` DISABLE KEYS */;
/*!40000 ALTER TABLE `configuration` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `configuration_fields`
--

DROP TABLE IF EXISTS `configuration_fields`;
CREATE TABLE `configuration_fields` (
  `plugin` tinyint(4) NOT NULL,
  `name` varchar(50) collate utf8_unicode_ci NOT NULL,
  `priority` tinyint(4) NOT NULL DEFAULT 0,
  `field_type` tinyint(4) NOT NULL,
  `description` varchar(100) collate utf8_unicode_ci NOT NULL,
  `default_value` varchar(250) collate utf8_unicode_ci default NULL,
  PRIMARY KEY  (`plugin`,`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Dumping data for table `configuration_fields`
--

LOCK TABLES `configuration_fields` WRITE;
/*!40000 ALTER TABLE `configuration_fields` DISABLE KEYS */;
/*!40000 ALTER TABLE `configuration_fields` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2008-05-30 19:34:35
