DROP TABLE IF EXISTS conference_jidstat CASCADE;
CREATE TABLE conference_jidstat (
  id SERIAL,
  jid_id int NOT NULL,

  time_online int NOT NULL default 0, 

  lastaction int NOT NULL default '0',
  lastreason varchar(200) NULL,

  msg_count int NOT NULL default 0,
  msg_chars int NOT NULL default 0,
  msg_words int NOT NULL default 0,
  msg_sentences int NOT NULL default 0,
  msg_me int NOT NULL default 0,
  msg_reply int NOT NULL default 0,
  msg_subject int NOT NULL default 0,

  cnt_join int NOT NULL default 0,
  cnt_leave int NOT NULL default 0,
  cnt_presence int NOT NULL default 0,
  cnt_nickchange int NOT NULL default 0,
  cnt_visitor int NOT NULL default 0,
  cnt_participant int NOT NULL default 0,
  cnt_moderator int NOT NULL default 0,
  cnt_noaffiliation int NOT NULL default 0,
  cnt_member int NOT NULL default 0,
  cnt_administrator int NOT NULL default 0,
  cnt_owner int NOT NULL default 0,
  cnt_kick int NOT NULL default 0,
  cnt_ban int NOT NULL default 0,
  
  ver_name varchar(50) NULL,
  ver_version varchar(50) NULL,
  ver_os varchar(200) NULL,

  PRIMARY KEY(id),
  FOREIGN KEY(jid_id) REFERENCES conference_jids(id) ON DELETE CASCADE,
  UNIQUE(jid_id)
);

