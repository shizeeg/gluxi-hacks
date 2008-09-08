DROP TABLE IF EXISTS roster;
CREATE TABLE roster (
  id SERIAL,
  jid varchar(100) NOT NULL,
  PRIMARY KEY(id),
  UNIQUE(jid)
);
