DROP TABLE IF EXISTS conference_jidstat_time CASCADE;
CREATE TABLE conference_jidstat_time (
  id SERIAL,
  jid_id int NOT NULL,

  int_type int NOT NULL,
  int_idx int NOT NULL,
  value int NOT NULL default 0,
  
  PRIMARY KEY(id),
  FOREIGN KEY(jid_id) REFERENCES conference_jids(id) ON DELETE CASCADE,
  UNIQUE(jid_id, int_type, int_idx)
);

