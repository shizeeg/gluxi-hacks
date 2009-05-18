DROP TABLE IF EXISTS conference_jidstat CASCADE;
CREATE TABLE conference_jidstat (
  id SERIAL,
  jid_id int NOT NULL,
  lastaction int NOT NULL default '0',
  lastreason varchar(200) NULL,
/*
  cnt_ban int NOT NULL default 0,
  cnt_visitor int NOT NULL default 0,
  cnt_participant int NOT NULL default 0,
  cnt_member int NOT NULL default 0,
  cnt_moderator int NOT NULL default 0,
  cnt_owner int NOT NULL default 0,
  cnt_statuschange int NOT NULL default 0,
  cnt_sentence int NOT NULL default 0,
  cnt_word int NOT NULL default 0,
*/
  PRIMARY KEY(id),
  FOREIGN KEY(jid_id) REFERENCES conference_jids(id) ON DELETE CASCADE,
  UNIQUE(jid_id)
);

