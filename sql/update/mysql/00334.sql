DROP TABLE IF EXISTS `acl`;
CREATE TABLE `acl` (
  `name` varchar(200) collate utf8_bin NOT NULL,
  `value` varchar(200) collate utf8_bin NOT NULL,
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
