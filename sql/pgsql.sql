DROP TABLE IF EXISTS aliases CASCADE;
CREATE TABLE aliases (
  plugin smallint NOT NULL,
  storage smallint NOT NULL,
  global boolean NOT NULL DEFAULT false,
  name varchar(50) NOT NULL,
  value varchar(200) NOT NULL,
  PRIMARY KEY (plugin,storage,name)
);

DROP TABLE IF EXISTS acl CASCADE;
CREATE TABLE acl (
  name varchar(2000) NOT NULL,
  value varchar(200) NOT NULL,
  PRIMARY KEY (name)
);

DROP TABLE IF EXISTS conference_alists CASCADE;
CREATE TABLE conference_alists (
  id SERIAL,
  conference_id int NOT NULL,
  list smallint NOT NULL,
  matcher smallint NOT NULL DEFAULT 0,
  test smallint NOT NULL DEFAULT 0,
  inv boolean NOT NULL DEFAULT false,
  value varchar(50) NOT NULL,
  child_id int NOT NULL DEFAULT 0,
  reason varchar(100) NULL,
  expire timestamp default NULL,
  PRIMARY KEY (id),
  UNIQUE (conference_id,list,matcher,test,value, child_id)
);

DROP TABLE IF EXISTS conference_jids CASCADE;
CREATE TABLE conference_jids (
  id SERIAL,
  conference_id int NOT NULL,
  jid varchar(50) NOT NULL,
  resource varchar(50) default NULL,
  temporary boolean NOT NULL default false,
  created timestamp default NULL,
  PRIMARY KEY (id)
);

DROP TABLE IF EXISTS conference_nicks CASCADE;
CREATE TABLE conference_nicks (
  id SERIAL,
  conference_id int NOT NULL,
  nick varchar(50) NOT NULL,
  jid int NOT NULL default '0',
  created timestamp NOT NULL,
  online boolean NOT NULL default false,
  joined timestamp NOT NULL,
  lastaction timestamp NOT NULL,
  PRIMARY KEY (id)
);

DROP TABLE IF EXISTS conferences CASCADE;
CREATE TABLE conferences (
  name varchar(50) NOT NULL,
  id SERIAL,
  nick varchar(50) NOT NULL,
  created timestamp NOT NULL,
  autojoin boolean NOT NULL default true,
  online boolean NOT NULL default false,
  joined timestamp default NULL,
  autoleave boolean NOT NULL default true,
  owner varchar(200) NULL,
  PRIMARY KEY (name),
  UNIQUE (id)
);

DROP TABLE IF EXISTS webstatus CASCADE;
CREATE TABLE webstatus (
  jid varchar(50) NOT NULL,
  hash varchar(50) NOT NULL,
  status varchar(15) default NULL,
  display varchar(300) default NULL,
  available varchar(100) default NULL,
  away varchar(100) default NULL,
  chat varchar(100) default NULL,
  dnd varchar(100) default NULL,
  unavailable varchar(100) default NULL,
  xa varchar(100) default NULL,
  PRIMARY KEY (jid),
  UNIQUE (hash)
);

DROP TABLE IF EXISTS words CASCADE;
CREATE TABLE words (
  plugin smallint NOT NULL,
  storage smallint NOT NULL,
  name varchar(50) NOT NULL,
  nick varchar(50) NOT NULL,
  date timestamp NOT NULL,
  value text NOT NULL,
  PRIMARY KEY (plugin,storage,name,nick)
);

DROP TABLE IF EXISTS configuration CASCADE;
CREATE TABLE configuration (
  plugin smallint NOT NULL,
  storage smallint NOT NULL,
  name varchar(50) NOT NULL,
  value varchar(250) NOT NULL,
  PRIMARY KEY (plugin,storage,name)
);

DROP TABLE IF EXISTS configuration_fields;
CREATE TABLE configuration_fields (
  plugin smallint NOT NULL,
  name varchar(50) NOT NULL,
  priority smallint NOT NULL DEFAULT 0,
  field_type smallint NOT NULL,
  description varchar(100) NOT NULL,
  default_value varchar(250) NULL DEFAULT NULL,
  PRIMARY KEY (plugin,name)
);

DROP TABLE IF EXISTS roster;
CREATE TABLE roster (
  id SERIAL,
  jid varchar(100) NOT NULL,
  PRIMARY KEY(id),
  UNIQUE(jid)
);

DROP TABLE IF EXISTS version CASCADE;
CREATE TABLE version (
  name varchar(50) NOT NULL,
  value varchar(50) NOT NULL,
  PRIMARY KEY (name)
);

INSERT INTO version (name, value) VALUES ('dbversion', 334);
