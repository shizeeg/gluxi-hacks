CREATE TABLE aliases (
  plugin smallint NOT NULL,
  storage smallint NOT NULL,
  name varchar(50) NOT NULL,
  value varchar(200) NOT NULL,
  PRIMARY KEY (plugin,storage,name)
);

CREATE TABLE conference_alists (
  id SERIAL,
  conference_id int NOT NULL,
  list smallint NOT NULL,
  matcher smallint NOT NULL DEFAULT 0,
  isregexp boolean NOT NULL DEFAULT false,
  value varchar(50) NOT NULL,
  reason varchar(100) NULL,
  expire timestamp default NULL,
  PRIMARY KEY (id),
  UNIQUE (conference_id,list,matcher,regexp,value)
);


CREATE TABLE conference_jids (
  id SERIAL,
  conference_id int NOT NULL,
  jid varchar(50) NOT NULL,
  resource varchar(50) default NULL,
  temporary boolean NOT NULL default false,
  created timestamp default NULL,
  PRIMARY KEY (id)
);

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

CREATE TABLE conferences (
  name varchar(50) NOT NULL,
  id SERIAL,
  nick varchar(50) NOT NULL,
  created timestamp NOT NULL,
  autojoin boolean NOT NULL default true,
  online boolean NOT NULL default false,
  joined timestamp default NULL,
  PRIMARY KEY (name),
  UNIQUE (id)
);

CREATE TABLE webstatus (
  jid varchar(50) NOT NULL,
  hash varchar(50) NOT NULL,
  status varchar(15) default NULL,
  available varchar(100) default NULL,
  away varchar(100) default NULL,
  chat varchar(100) default NULL,
  dnd varchar(100) default NULL,
  unavailable varchar(100) default NULL,
  xa varchar(100) default NULL,
  PRIMARY KEY (jid),
  UNIQUE (hash)
);


CREATE TABLE words (
  plugin smallint NOT NULL,
  storage smallint NOT NULL,
  name varchar(50) NOT NULL,
  nick varchar(50) NOT NULL,
  date timestamp NOT NULL,
  value text NOT NULL,
  PRIMARY KEY (plugin,storage,name,nick)
);
