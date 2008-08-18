DROP TABLE IF EXISTS acl CASCADE;
CREATE TABLE acl (
  name varchar(2000) NOT NULL,
  value varchar(200) NOT NULL,
  PRIMARY KEY (name)
);
