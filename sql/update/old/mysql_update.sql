alter table conference_alists drop primary key;
alter table conference_alists add id int(10) NOT NULL auto_increment FIRST, add PRIMARY KEY(id);
alter table conference_alists add `matcher` tinyint(4) NOT NULL DEFAULT '0' AFTER `list`;
alter table conference_alists add `isregexp` tinyint(1) NOT NULL DEFAULT '0' AFTER `matcher`;
alter table conference_alists add `reason` varchar(100) collate utf8_bin NULL AFTER `value`;
alter table conference_alists add key (`conference_id`,`list`,`matcher`, `isregexp`, `value`);

alter table conference_alists add `test` tinyint(4) NOT NULL DEFAULT '0' AFTER `matcher`;
update conference_alists set test=2 where isregexp=true;
update conference_alists set test=1 where isregexp=false;
alter table conference_alists drop isregexp;
alter table conference_alists add `inv` tinyint(1) NOT NULL DEFAULT '0' AFTER `test`;


alter table aliases add global tinyint(1) not null default '0' after `storage`; 

alter table conferences add autoleave tinyint(1) NOT NULL default '1' after `joined`;
alter table conferences add owner varchar(200) NULL after `autoleave`;

